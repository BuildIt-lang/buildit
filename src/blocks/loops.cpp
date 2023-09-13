#include "blocks/loops.h"
#include <algorithm>
#include <tuple>

static std::vector<std::tuple<unsigned int, std::reference_wrapper<std::vector<stmt::Ptr>>, stmt::Ptr>> worklist;

std::shared_ptr<loop> loop_info::allocate_loop(std::shared_ptr<basic_block> header) {
    if (!header)
        return nullptr;

    loops.push_back(std::make_shared<loop>(header));
    bb_loop_map[header->id] = loops.back();
    return loops.back();
}

void loop_info::postorder_dfs_helper(std::vector<int> &postorder_loops_map, std::vector<bool> &visited_loops, int id) {
    for (auto subloop: loops[id]->subloops) {
        if (!visited_loops[subloop->loop_id]) {
            visited_loops[subloop->loop_id] = true;
            postorder_dfs_helper(postorder_loops_map, visited_loops, subloop->loop_id);
            postorder_loops_map.push_back(subloop->loop_id);
        }
    }
}

void loop_info::preorder_dfs_helper(std::vector<int> &preorder_loops_map, std::vector<bool> &visited_loops, int id) {
    for (auto subloop: loops[id]->subloops) {
        if (!visited_loops[subloop->loop_id]) {
            visited_loops[subloop->loop_id] = true;
            preorder_loops_map.push_back(subloop->loop_id);
            preorder_dfs_helper(preorder_loops_map, visited_loops, subloop->loop_id);
        }
    }
}

void loop_info::analyze() {
    std::vector<int> idom = dta.get_idom();

    for (int idom_id: dta.get_postorder_idom_map()) {
        std::vector<int> backedges;
        int header = idom_id;

        for (auto backedge: dta.cfg_[header]->predecessor) {
            if (dta.dominates(header, backedge->id) && dta.is_reachable_from_entry(backedge->id)) {
                backedges.push_back(backedge->id);
            }
        }

        if (!backedges.empty()) {
            std::shared_ptr<loop> new_loop = allocate_loop(dta.cfg_[header]);
            if (!new_loop)
                continue;

            int num_blocks = 0;
            int num_subloops = 0;

            auto backedge_iter = backedges.begin();
            // do a reverse CFG traversal to map basic blocks in this loop.
            basic_block::cfg_block worklist(backedges.size());
            std::generate(worklist.begin(), worklist.end(), [&backedge_iter, this](){
                return dta.cfg_[*(backedge_iter++)];
            });

            while (!worklist.empty()) {
                unsigned int predecessor_bb_id = worklist.back()->id;
                worklist.pop_back();

                auto subloop_iter = bb_loop_map.find(predecessor_bb_id);
                if (subloop_iter == bb_loop_map.end()) {
                    if (!dta.is_reachable_from_entry(predecessor_bb_id))
                        continue;

                    bb_loop_map[predecessor_bb_id] = new_loop;
                    ++num_blocks;
                    // loop has no blocks between header and backedge
                    if (predecessor_bb_id == new_loop->header_block->id)
                        continue;

                    worklist.insert(worklist.end(), dta.cfg_[predecessor_bb_id]->predecessor.begin(), dta.cfg_[predecessor_bb_id]->predecessor.end());
                }
                else {
                    // this block has already been discovered, mapped to some other loop
                    // find the outermost loop
                    std::shared_ptr<loop> subloop = subloop_iter->second;
                    while (subloop->parent_loop) {
                        subloop = subloop->parent_loop;
                    }

                    if (subloop == new_loop)
                        continue;

                    // discovered a subloop of this loop
                    subloop->parent_loop = new_loop;
                    ++num_subloops;
                    num_blocks = num_blocks + subloop->blocks.size();
                    predecessor_bb_id = subloop->header_block->id;

                    for (auto pred: dta.cfg_[predecessor_bb_id]->predecessor) {
                        auto loop_iter = bb_loop_map.find(pred->id);
                        // do not check if loop_iter != bb_loop_map.end(), as a results
                        // basic blocks that are not directly part of the natural loops
                        // are skipped, like loop latches.
                        if (loop_iter->second != subloop)
                            worklist.push_back(pred);
                    }
                }
            }
            new_loop->subloops.reserve(num_subloops);
            new_loop->blocks.reserve(num_blocks);
            new_loop->blocks_id_map.reserve(num_blocks);
        }
    }

    // populate all subloops and loops with blocks
    for (auto bb_id: dta.get_postorder()) {
        auto subloop_iter = bb_loop_map.find(bb_id);
        std::shared_ptr<loop> subloop = nullptr;
        if (subloop_iter != bb_loop_map.end() && (subloop = subloop_iter->second) && dta.cfg_[bb_id] == subloop_iter->second->header_block) {
            // check if it is the outermost loop
            if (subloop->parent_loop != nullptr) {
                subloop->parent_loop->subloops.push_back(subloop);
            }
            else {
                top_level_loops.push_back(subloop);
            }

            std::reverse(subloop->blocks.begin(), subloop->blocks.end());
            std::reverse(subloop->subloops.begin(), subloop->subloops.end());

            subloop = subloop->parent_loop;
        }

        while (subloop) {
            subloop->blocks.push_back(dta.cfg_[bb_id]);
            subloop->blocks_id_map.insert(dta.cfg_[bb_id]->id);
            subloop = subloop->parent_loop;
        }
    }

    // Populate the loop latches
    for (auto loop: loops) {
        if (!loop->header_block)
            continue;

        std::shared_ptr<basic_block> header = loop->header_block;
        for (auto children: header->predecessor) {
            if (loop->blocks_id_map.count(children->id)) {
                loop->loop_latch_blocks.push_back(children);
            }
        }
    }

    // Populate the loop exits
    for (auto loop: loops) {
        if (!loop->header_block)
            continue;
        
        for (auto bb: loop->blocks) {
            for (auto children: bb->successor) {
                if (!loop->blocks_id_map.count(children->id) && children->id != loop->header_block->id) {
                    loop->loop_exit_blocks.push_back(bb);
                    break;
                }
            }
        }
    }

    // Update unique loop exit using post dominators
    for (auto loop: loops) {
        if (loop->loop_exit_blocks.size() == 0)
            continue;

        int unique_postdom = post_dta.get_idom(loop->loop_exit_blocks[0]->id);
        bool unique_postdom_flag = true;
        for (auto exit_bb: loop->loop_exit_blocks) {
            if (post_dta.get_idom(exit_bb->id) != unique_postdom) {
                unique_postdom_flag = false;
            }
        }

        if (unique_postdom_flag)
            loop->unique_exit_block = dta.cfg_[unique_postdom];
    }

    // Assign id to the loops
    for (unsigned int i = 0; i < loops.size(); i++) {
        loops[i]->loop_id = i;
    }

    // build a postorder loop tree
    std::vector<bool> visited_loops(loops.size());
    visited_loops.assign(visited_loops.size(), false);
    for (auto loop: top_level_loops) {
        std::vector<int> postorder_loop_tree;
        visited_loops[loop->loop_id] = true;

        postorder_dfs_helper(postorder_loop_tree, visited_loops, loop->loop_id);
        postorder_loop_tree.push_back(loop->loop_id);
        postorder_loops_map[loop->loop_id] = postorder_loop_tree;
    }

    // build a preorder loop tree
    visited_loops.clear();
    visited_loops.assign(visited_loops.size(), false);
    for (auto loop: top_level_loops) {
        std::vector<int> preorder_loop_tree;
        visited_loops[loop->loop_id] = true;

        preorder_loop_tree.push_back(loop->loop_id);
        preorder_dfs_helper(preorder_loop_tree, visited_loops, loop->loop_id);
        preorder_loops_map[loop->loop_id] = preorder_loop_tree;
    }
}

static stmt::Ptr get_loop_block(std::shared_ptr<basic_block> bb, block::stmt_block::Ptr ast, bool one_level_up = false) {
    block::stmt::Ptr current_ast = to<block::stmt>(ast);
    std::vector<stmt::Ptr> current_block = to<block::stmt_block>(current_ast)->stmts;
    std::deque<stmt::Ptr> worklist;
    std::map<stmt::Ptr, stmt::Ptr> ast_parent_map;
    // std::cerr << bb->name << "\n";

    for (auto stmt: current_block) {
        ast_parent_map[stmt] = current_ast;
    }
    worklist.insert(worklist.end(), current_block.begin(), current_block.end());

    while (worklist.size()) {
        stmt::Ptr worklist_top = worklist.front();
        worklist.pop_front();

        if (isa<block::stmt_block>(worklist_top)) {
            stmt_block::Ptr wl_stmt_block = to<stmt_block>(worklist_top);
            for (auto stmt_: wl_stmt_block->stmts) {
                ast_parent_map[stmt_] = worklist_top;
            }
            worklist.insert(worklist.end(), wl_stmt_block->stmts.begin(), wl_stmt_block->stmts.end());

            if (worklist_top == bb->parent)
                return ast_parent_map[worklist_top];
        }
        else if (isa<block::if_stmt>(worklist_top)) {
            if_stmt::Ptr wl_if_stmt = to<if_stmt>(worklist_top);
            // std::cerr << "found if: ";
            if (to<stmt_block>(wl_if_stmt->then_stmt)->stmts.size() != 0) {
                // std::cerr << "then\n";
                stmt_block::Ptr wl_if_then_stmt = to<stmt_block>(wl_if_stmt->then_stmt);
                for (auto stmt_: wl_if_then_stmt->stmts) {
                    ast_parent_map[stmt_] = worklist_top;
                }
                worklist.insert(worklist.end(), wl_if_then_stmt->stmts.begin(), wl_if_then_stmt->stmts.end());
            }
            if (to<stmt_block>(wl_if_stmt->else_stmt)->stmts.size() != 0) {
                // std::cerr << "else\n";
                stmt_block::Ptr wl_if_else_stmt = to<stmt_block>(wl_if_stmt->else_stmt);
                for (auto stmt_: wl_if_else_stmt->stmts) {
                    ast_parent_map[stmt_] = worklist_top;
                }
                worklist.insert(worklist.end(), wl_if_else_stmt->stmts.begin(), wl_if_else_stmt->stmts.end());
            }

            if (worklist_top == bb->parent)
                return one_level_up ? ast_parent_map[ast_parent_map[worklist_top]] : ast_parent_map[worklist_top];
        }
        else if (isa<block::while_stmt>(worklist_top)) {
            stmt_block::Ptr wl_while_body_block = to<stmt_block>(to<block::while_stmt>(worklist_top)->body);
            for (auto stmt: wl_while_body_block->stmts) {
                ast_parent_map[stmt] = worklist_top;
            }
            worklist.insert(worklist.end(), wl_while_body_block->stmts.begin(), wl_while_body_block->stmts.end());

            if (worklist_top == bb->parent)
                return one_level_up ? ast_parent_map[ast_parent_map[worklist_top]] : ast_parent_map[worklist_top];
        }
        else if (isa<block::label_stmt>(worklist_top)) {
            // std::cerr << "found label\n";
            if (worklist_top == bb->parent)
                return one_level_up ? ast_parent_map[ast_parent_map[worklist_top]] : ast_parent_map[worklist_top];
        }
        else if (isa<block::goto_stmt>(worklist_top)) {
            // std::cerr << "found goto\n";
            if (worklist_top == bb->parent)
                return one_level_up ? ast_parent_map[ast_parent_map[worklist_top]] : ast_parent_map[worklist_top];
        }
        else if (isa<block::expr_stmt>(worklist_top)) {
            if (worklist_top == bb->parent)
                return one_level_up ? ast_parent_map[ast_parent_map[worklist_top]] : ast_parent_map[worklist_top];
        }
    }

    std::cerr << "Returned nullptr !\n";
    return nullptr;
}

static void replace_loop_exits(std::shared_ptr<loop> loop, block::stmt_block::Ptr ast, dominator_analysis &dta_) {
    for (auto exit_bb: loop->loop_exit_blocks) {
        if (!loop->unique_exit_block)
            return;

        int target_branch = -1;

        if (!isa<if_stmt>(exit_bb->parent))
            assert(0);
        
        if (exit_bb->then_branch && !loop->blocks_id_map.count(exit_bb->then_branch->id))
            target_branch = 0;
        else if (exit_bb->else_branch && !loop->blocks_id_map.count(exit_bb->else_branch->id))
            target_branch = 1;

        if (target_branch == -1)
            assert(0);

        if (target_branch == 0) {
            std::vector<stmt::Ptr> &temp_ast = to<block::stmt_block>(to<if_stmt>(exit_bb->parent)->then_stmt)->stmts;

            if (exit_bb->then_branch == loop->unique_exit_block) {
                temp_ast.push_back(std::make_shared<block::break_stmt>());
            }
            else {
                std::shared_ptr<basic_block> exiting_bb = exit_bb->then_branch;
                std::cerr << exiting_bb->id << "\n";
                // if (!isa<stmt_block>(exiting_bb->parent) || to<stmt_block>(exiting_bb->parent)->stmts.size() == 0)
                //     exiting_bb = exit_bb->then_branch->successor[0];
                unsigned int preorder_index = dta_.get_preorder_bb_map()[exiting_bb->id];
                while(exiting_bb->is_exit_block) {
                    std::shared_ptr<basic_block> temp_exiting_bb = dta_.cfg_[dta_.get_preorder()[++preorder_index]];
                    if (temp_exiting_bb->id == loop->unique_exit_block->id)
                        break;
                    exiting_bb = temp_exiting_bb;
                }
                std::cerr << exiting_bb->id << "\n";
                
                stmt::Ptr loop_exit_ast;
                if (exiting_bb->is_exit_block)
                    loop_exit_ast = exit_bb->parent;
                else
                    loop_exit_ast = get_loop_block(exiting_bb, ast);

                if (isa<block::stmt_block>(loop_exit_ast)) {
                    to<block::stmt_block>(loop_exit_ast)->stmts.push_back(std::make_shared<block::break_stmt>());
                }
                else if (isa<block::if_stmt>(loop_exit_ast)) {
                    stmt_block::Ptr if_then_block = to<block::stmt_block>(to<block::if_stmt>(loop_exit_ast)->then_stmt);
                    // stmt_block::Ptr if_else_block = to<block::stmt_block>(to<block::if_stmt>(loop_exit_ast)->else_stmt);

                    // if (exiting_bb->ast_index < if_then_block->stmts.size() && if_then_block->stmts[exiting_bb->ast_index] == exiting_bb->parent)
                        if_then_block->stmts.push_back(std::make_shared<block::break_stmt>());
                    // else if (exiting_bb->ast_index < if_else_block->stmts.size() && if_else_block->stmts[exiting_bb->ast_index] == exiting_bb->parent)
                    //     if_else_block->stmts.push_back(std::make_shared<block::break_stmt>());
                }
            }
        }
        else if (target_branch == 1) {
            std::vector<stmt::Ptr> &temp_ast = to<block::stmt_block>(to<if_stmt>(exit_bb->parent)->else_stmt)->stmts;

            if (exit_bb->else_branch == loop->unique_exit_block) {
                temp_ast.push_back(std::make_shared<block::break_stmt>());
            }
            else {
                std::shared_ptr<basic_block> exiting_bb = exit_bb->else_branch;
                std::cerr << exiting_bb->id << "\n";
                // if (!isa<stmt_block>(exiting_bb->parent) || to<stmt_block>(exiting_bb->parent)->stmts.size() == 0)
                //     exiting_bb = exit_bb->else_branch->successor[0];
                unsigned int preorder_index = dta_.get_preorder_bb_map()[exiting_bb->id];
                while(exiting_bb->is_exit_block) {
                    std::shared_ptr<basic_block> temp_exiting_bb = dta_.cfg_[dta_.get_preorder()[++preorder_index]];
                    if (temp_exiting_bb->id == loop->unique_exit_block->id)
                        break;
                    exiting_bb = temp_exiting_bb;
                }
                std::cerr << exiting_bb->id << "\n";

                stmt::Ptr loop_exit_ast;
                if (exiting_bb->is_exit_block)
                    loop_exit_ast = exit_bb->parent;
                else
                    loop_exit_ast = get_loop_block(exiting_bb, ast);

                if (isa<block::stmt_block>(loop_exit_ast)) {
                    to<block::stmt_block>(loop_exit_ast)->stmts.push_back(std::make_shared<block::break_stmt>());
                }
                else if (isa<block::if_stmt>(loop_exit_ast)) {
                    // stmt_block::Ptr if_then_block = to<block::stmt_block>(to<block::if_stmt>(loop_exit_ast)->then_stmt);
                    stmt_block::Ptr if_else_block = to<block::stmt_block>(to<block::if_stmt>(loop_exit_ast)->else_stmt);

                    // if (exiting_bb->ast_index < if_then_block->stmts.size() && if_then_block->stmts[exiting_bb->ast_index] == exiting_bb->parent)
                    //     if_then_block->stmts.push_back(std::make_shared<block::break_stmt>());
                    // else if (exiting_bb->ast_index < if_else_block->stmts.size() && if_else_block->stmts[exiting_bb->ast_index] == exiting_bb->parent)
                        if_else_block->stmts.push_back(std::make_shared<block::break_stmt>());
                }
            }
        }                   
    }
}

static std::vector<stmt_block::Ptr> backedge_blocks;

static void replace_loop_latches(std::shared_ptr<loop> loop, block::stmt_block::Ptr ast, dominator_analysis &dta_) {
    std::cerr << "loop id: " << loop->loop_id << " : ";
    for (auto latch_iter = loop->loop_latch_blocks.begin(); latch_iter != loop->loop_latch_blocks.end(); latch_iter++) {
        std::cerr << "loop latch id: " << (*latch_iter)->id << "\n";
        stmt::Ptr loop_latch_ast = get_loop_block(*latch_iter, ast);
        // stmt::Ptr loop_latch_ast_level_up = get_loop_block(*latch_iter, ast, true);
        bool is_last_block = false;
        bool is_latch_in_other_loop = false;

        if (dta_.get_preorder_bb_map()[(*latch_iter)->id] == (int)dta_.get_preorder().size() - 1) {
            is_last_block = true;
        }
        else {
            int next_preorder = dta_.get_preorder_bb_map()[(*latch_iter)->id] + 1 < (int)dta_.get_preorder().size() ? dta_.get_preorder()[dta_.get_preorder_bb_map()[(*latch_iter)->id] + 1] : -1;
            int next_next_preorder = dta_.get_preorder_bb_map()[next_preorder] + 1 < (int)dta_.get_preorder().size() ? dta_.get_preorder()[dta_.get_preorder_bb_map()[next_preorder] + 1] : -1;

            std::cerr << next_preorder << " : " << next_next_preorder << "\n";
            if (loop->blocks_id_map.count(next_preorder))
                is_last_block = false;
            else {
                if (loop->unique_exit_block && (next_preorder == (int)loop->unique_exit_block->id))
                    is_last_block = true;
                else if (loop->unique_exit_block && (next_next_preorder == (int)loop->unique_exit_block->id))
                    is_last_block = true;
                else if (next_preorder != -1 && next_next_preorder == -1 && dta_.cfg_[next_preorder]->is_exit_block)
                    is_last_block = true;
                else if (next_preorder != -1 && next_next_preorder != -1 && dta_.cfg_[next_preorder]->is_exit_block && isa<goto_stmt>(dta_.cfg_[next_next_preorder]->parent))
                    is_last_block = true;
                else
                    is_last_block = false;
            }
        }
        std::cerr << "==== ast ====\n";
        loop_latch_ast->dump(std::cerr, 0);
        if ((*latch_iter)->ast_to_basic_block_map.count(loop_latch_ast)) {
            std::cerr << "break in latch: " << (*latch_iter)->ast_to_basic_block_map.at(loop_latch_ast)->id << "\n";
            auto bb = (*latch_iter)->ast_to_basic_block_map.at(loop_latch_ast);
            for (auto subloop: loop->subloops) {
                if (subloop->blocks_id_map.count(bb->id))
                    is_latch_in_other_loop = true;
            }
        }

        std::cerr << "LL: " << (*latch_iter)->id << ": " << is_last_block  << ": " << is_latch_in_other_loop << "\n";
        if (isa<stmt_block>(loop_latch_ast)) {
            std::cerr << "stmt parent\n";
            std::vector<stmt::Ptr> &temp_ast = to<block::stmt_block>(loop_latch_ast)->stmts;
            if (is_last_block)
                temp_ast.erase(temp_ast.begin() + (*latch_iter)->ast_index);
            else {
                backedge_blocks.push_back(to<stmt_block>(loop_latch_ast));
                std::replace(temp_ast.begin(), temp_ast.end(), temp_ast[(*latch_iter)->ast_index + 1], is_latch_in_other_loop ? to<stmt>(std::make_shared<break_stmt>()) : to<stmt>(std::make_shared<continue_stmt>()));
            }
        }
        else if (isa<if_stmt>(loop_latch_ast)) {
            stmt_block::Ptr if_then_block = to<block::stmt_block>(to<block::if_stmt>(loop_latch_ast)->then_stmt);
            stmt_block::Ptr if_else_block = to<block::stmt_block>(to<block::if_stmt>(loop_latch_ast)->else_stmt);
            
            std::cerr << "if parent: ";
            std::cerr << if_then_block->stmts.size() << " " << if_else_block->stmts.size() << " " << (*latch_iter)->ast_index << " ";
            if (if_then_block->stmts.size() && if_then_block->stmts[(*latch_iter)->ast_index] == (*latch_iter)->parent) {
                std::cerr << "then\n";
                std::vector<stmt::Ptr> &temp_ast = if_then_block->stmts;
                if (is_last_block)
                    temp_ast.erase(temp_ast.begin() + (*latch_iter)->ast_index);
                else {
                    backedge_blocks.push_back(if_then_block);
                    std::replace(temp_ast.begin(), temp_ast.end(), temp_ast[(*latch_iter)->ast_index], is_latch_in_other_loop ? to<stmt>(std::make_shared<break_stmt>()) : to<stmt>(std::make_shared<continue_stmt>()));
                }
            }
            
            if (if_else_block->stmts.size() && if_else_block->stmts[(*latch_iter)->ast_index] == (*latch_iter)->parent) {
                std::cerr << "else\n";
                std::vector<stmt::Ptr> &temp_ast = if_else_block->stmts;
                if (is_last_block)
                    temp_ast.erase(temp_ast.begin() + (*latch_iter)->ast_index);
                else {
                    backedge_blocks.push_back(if_else_block);
                    std::replace(temp_ast.begin(), temp_ast.end(), temp_ast[(*latch_iter)->ast_index], is_latch_in_other_loop ? to<stmt>(std::make_shared<break_stmt>()) : to<stmt>(std::make_shared<continue_stmt>()));
                }
            }
        }
    }
}

block::stmt_block::Ptr loop_info::convert_to_ast(block::stmt_block::Ptr ast) {
    std::cerr << "before ast\n";
    ast->dump(std::cerr, 0);
    std::cerr << "before ast\n";

    for (auto loop_map: preorder_loops_map) {
        for (auto preorder: loop_map.second) {
            replace_loop_exits(loops[preorder], ast, dta);
            replace_loop_latches(loops[preorder], ast, dta);
        }
    }

    std::cerr << "after ast\n";
    ast->dump(std::cerr, 0);
    std::cerr << "after ast\n";

    // return ast;
    for (auto loop_map: postorder_loops_map) {
        for (auto postorder: loop_map.second) {
            std::cerr << "before ast\n";
            ast->dump(std::cerr, 0);
            std::cerr << "before ast\n";
            block::stmt::Ptr loop_header_ast = get_loop_block(loops[postorder]->header_block, ast);
            std::cerr << loops[postorder]->header_block->ast_index << "\n";
            loop_header_ast->dump(std::cerr, 0);
            while_stmt::Ptr while_block = std::make_shared<while_stmt>();
            while_block->body = std::make_shared<stmt_block>();

            if (isa<block::while_stmt>(loop_header_ast)) {
                loop_header_ast = to<block::while_stmt>(loop_header_ast)->body;
            }

            if (isa<block::stmt_block>(loop_header_ast)) {
                unsigned int ast_index = loops[postorder]->header_block->ast_index;
                // handle unconditional loops
                if (to<block::stmt_block>(loop_header_ast)->stmts[ast_index] == loops[postorder]->header_block->parent && !isa<if_stmt>(to<block::stmt_block>(loop_header_ast)->stmts[ast_index + 1])) {
                    for (auto bb: loops[postorder]->blocks) {
                        to<stmt_block>(while_block->body)->stmts.push_back(bb->parent);
                    }
                    // pop loop backedge
                    to<stmt_block>(while_block->body)->stmts.pop_back();

                    // set always true condition
                    while_block->cond = std::make_shared<int_const>();
                    to<int_const>(while_block->cond)->value = 1;

                    // unconditional loops can have only one backedge !?
                    assert(loops[postorder]->loop_latch_blocks.size() == 1);
                    for (unsigned int i = ast_index + 2; i < loops[postorder]->loop_latch_blocks[0]->ast_index; i++) {
                        std::cerr << i << "\n";
                        worklist.push_back(std::make_tuple(i, std::ref(to<block::stmt_block>(loop_header_ast)->stmts), nullptr));
                    }

                    worklist.push_back(std::make_tuple(ast_index, std::ref(to<block::stmt_block>(loop_header_ast)->stmts), to<stmt>(while_block)));
                }
                else if (to<block::stmt_block>(loop_header_ast)->stmts[ast_index] == loops[postorder]->header_block->parent) {
                    stmt_block::Ptr then_block = to<block::stmt_block>(to<block::if_stmt>(to<block::stmt_block>(loop_header_ast)->stmts[ast_index + 1])->then_stmt);
                    stmt_block::Ptr else_block = to<block::stmt_block>(to<block::if_stmt>(to<block::stmt_block>(loop_header_ast)->stmts[ast_index + 1])->else_stmt);
                    std::cerr << "stmt block\n";

                    while_block->cond = to<block::if_stmt>(to<block::stmt_block>(loop_header_ast)->stmts[ast_index + 1])->cond;
                    if (then_block->stmts.size() == 0 && else_block->stmts.size() != 0) {
                        not_expr::Ptr new_cond = std::make_shared<not_expr>();
                        new_cond->static_offset = while_block->cond->static_offset;
                        new_cond->expr1 = while_block->cond;
                        while_block->cond = new_cond;

                        for (auto body_stmt: else_block->stmts) {
                            to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                        }

                        auto backedge_iter = std::find(backedge_blocks.begin(), backedge_blocks.end(), else_block);
                        if (backedge_iter != backedge_blocks.end()) {
                            std::cerr << "replaced BE\n";
                            std::replace(backedge_blocks.begin(), backedge_blocks.end(), *backedge_iter, to<stmt_block>(while_block->body));
                        }
                        while_block->continue_blocks.insert(while_block->continue_blocks.begin(), backedge_blocks.begin(), backedge_blocks.end());
                    }
                    else if (then_block->stmts.size() <= 2 && isa<block::break_stmt>(then_block->stmts.back())) {
                        not_expr::Ptr new_cond = std::make_shared<not_expr>();
                        new_cond->static_offset = while_block->cond->static_offset;
                        new_cond->expr1 = while_block->cond;
                        while_block->cond = new_cond;

                        // if (else_block->stmts.size() != 0)
                            for (auto body_stmt: else_block->stmts) {
                                to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                            }
                        // else {
                            then_block->stmts.pop_back();
                            for (auto stmt: then_block->stmts)
                                to<block::stmt_block>(loop_header_ast)->stmts.push_back(stmt);
                        // }

                        auto backedge_iter = std::find(backedge_blocks.begin(), backedge_blocks.end(), else_block);
                        if (backedge_iter != backedge_blocks.end()) {
                            std::cerr << "replaced BE\n";
                            std::replace(backedge_blocks.begin(), backedge_blocks.end(), *backedge_iter, to<stmt_block>(while_block->body));
                        }
                        while_block->continue_blocks.insert(while_block->continue_blocks.begin(), backedge_blocks.begin(), backedge_blocks.end());
                    }
                    // else if (then_block->stmts.size() <= 2 && isa<block::break_stmt>(then_block->stmts.back())) {
                    //     not_expr::Ptr new_cond = std::make_shared<not_expr>();
                    //     new_cond->static_offset = while_block->cond->static_offset;
                    //     new_cond->expr1 = while_block->cond;
                    //     while_block->cond = new_cond;

                    //     then_block->stmts.pop_back();
                    //     for (auto stmt: then_block->stmts)
                    //         to<block::stmt_block>(loop_header_ast)->stmts.push_back(stmt);
                    // }
                    else {
                        for (auto body_stmt: then_block->stmts) {
                            to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                        }

                        auto backedge_iter = std::find(backedge_blocks.begin(), backedge_blocks.end(), then_block);
                        if (backedge_iter != backedge_blocks.end()) {
                            std::cerr << "replaced BE\n";
                            std::replace(backedge_blocks.begin(), backedge_blocks.end(), *backedge_iter, to<stmt_block>(while_block->body));
                        }
                        while_block->continue_blocks.insert(while_block->continue_blocks.begin(), backedge_blocks.begin(), backedge_blocks.end());
                    }
                    // for (auto body_stmt: else_block->stmts) {
                    //     to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                    // }
                    // if block to be replaced with while block
                    worklist.push_back(std::make_tuple(ast_index, std::ref(to<block::stmt_block>(loop_header_ast)->stmts), to<stmt>(while_block)));
                }
            }
            else if (isa<block::if_stmt>(loop_header_ast)) {
                unsigned int ast_index = loops[postorder]->header_block->ast_index;
                stmt_block::Ptr if_then_block = to<block::stmt_block>(to<block::if_stmt>(loop_header_ast)->then_stmt);
                stmt_block::Ptr if_else_block = to<block::stmt_block>(to<block::if_stmt>(loop_header_ast)->else_stmt);

                if (if_then_block->stmts.size() != 0) {
                    std::cerr << "if then block\n";
                    // handle unconditional loops
                    if (if_then_block->stmts[ast_index] == loops[postorder]->header_block->parent && !isa<block::if_stmt>(if_then_block->stmts[ast_index + 1])) {
                        for (auto bb: loops[postorder]->blocks) {
                            to<stmt_block>(while_block->body)->stmts.push_back(bb->parent);
                        }
                        // pop loop backedge
                        to<stmt_block>(while_block->body)->stmts.pop_back();

                        // set always true condition
                        while_block->cond = std::make_shared<int_const>();
                        to<int_const>(while_block->cond)->value = 1;

                        // unconditional loops can have only one backedge !?
                        assert(loops[postorder]->loop_latch_blocks.size() == 1);
                        for (unsigned int i = ast_index + 2; i < loops[postorder]->loop_latch_blocks[0]->ast_index; i++) {
                            worklist.push_back(std::make_tuple(i, std::ref(if_then_block->stmts), nullptr));
                        }

                        worklist.push_back(std::make_tuple(ast_index, std::ref(if_then_block->stmts), to<stmt>(while_block)));
                    }
                    else if (if_then_block->stmts[ast_index] == loops[postorder]->header_block->parent) {
                        stmt_block::Ptr then_block = to<block::stmt_block>(to<block::if_stmt>(if_then_block->stmts[ast_index + 1])->then_stmt);
                        stmt_block::Ptr else_block = to<block::stmt_block>(to<block::if_stmt>(if_then_block->stmts[ast_index + 1])->else_stmt);

                        while_block->cond = to<block::if_stmt>(if_then_block->stmts[ast_index + 1])->cond;
                        if (then_block->stmts.size() == 0 && else_block->stmts.size() != 0) {
                            not_expr::Ptr new_cond = std::make_shared<not_expr>();
				            new_cond->static_offset = while_block->cond->static_offset;
				            new_cond->expr1 = while_block->cond;
				            while_block->cond = new_cond;

                            for (auto body_stmt: else_block->stmts) {
                                to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                            }

                            auto backedge_iter = std::find(backedge_blocks.begin(), backedge_blocks.end(), else_block);
                            if (backedge_iter != backedge_blocks.end()) {
                                std::cerr << "replaced BE\n";
                                std::replace(backedge_blocks.begin(), backedge_blocks.end(), *backedge_iter, to<stmt_block>(while_block->body));
                            }
                            while_block->continue_blocks.insert(while_block->continue_blocks.begin(), backedge_blocks.begin(), backedge_blocks.end());
                        }
                        else if (then_block->stmts.size() <= 2 && isa<block::break_stmt>(then_block->stmts.back())) {
                            not_expr::Ptr new_cond = std::make_shared<not_expr>();
                            new_cond->static_offset = while_block->cond->static_offset;
                            new_cond->expr1 = while_block->cond;
                            while_block->cond = new_cond;

                            for (auto body_stmt: else_block->stmts) {
                                to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                            }

                            then_block->stmts.pop_back();
                            for (auto stmt: then_block->stmts)
                                if_then_block->stmts.push_back(stmt);

                            auto backedge_iter = std::find(backedge_blocks.begin(), backedge_blocks.end(), else_block);
                            if (backedge_iter != backedge_blocks.end()) {
                                std::cerr << "replaced BE\n";
                                std::replace(backedge_blocks.begin(), backedge_blocks.end(), *backedge_iter, to<stmt_block>(while_block->body));
                            }
                            while_block->continue_blocks.insert(while_block->continue_blocks.begin(), backedge_blocks.begin(), backedge_blocks.end());
                        }
                        // else if (then_block->stmts.size() <= 2 && isa<block::break_stmt>(then_block->stmts.back())) {
                        //     not_expr::Ptr new_cond = std::make_shared<not_expr>();
                        //     new_cond->static_offset = while_block->cond->static_offset;
                        //     new_cond->expr1 = while_block->cond;
                        //     while_block->cond = new_cond;

                        //     then_block->stmts.pop_back();
                        //     for (auto stmt: then_block->stmts)
                        //         if_then_block->stmts.push_back(stmt);
                        // }
                        else {
                            for (auto body_stmt: then_block->stmts) {
                                to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                            }

                            auto backedge_iter = std::find(backedge_blocks.begin(), backedge_blocks.end(), then_block);
                            if (backedge_iter != backedge_blocks.end()) {
                                std::cerr << "replaced BE\n";
                                std::replace(backedge_blocks.begin(), backedge_blocks.end(), *backedge_iter, to<stmt_block>(while_block->body));
                            }
                            while_block->continue_blocks.insert(while_block->continue_blocks.begin(), backedge_blocks.begin(), backedge_blocks.end());
                        }
                        // for (auto body_stmt: else_block->stmts) {
                        //     to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                        // }
                        // if block to be replaced with while block
                        worklist.push_back(std::make_tuple(ast_index, std::ref(if_then_block->stmts), to<stmt>(while_block)));
                    }
                }
                
                if (if_else_block->stmts.size() != 0) {
                    std::cerr << "if else block\n";
                    // handle unconditional loops
                    if (if_else_block->stmts[ast_index] == loops[postorder]->header_block->parent && !isa<block::if_stmt>(if_else_block->stmts[ast_index + 1])) {
                        for (auto bb: loops[postorder]->blocks) {
                            to<stmt_block>(while_block->body)->stmts.push_back(bb->parent);
                        }
                        // pop loop backedge
                        to<stmt_block>(while_block->body)->stmts.pop_back();

                        // set always true condition
                        while_block->cond = std::make_shared<int_const>();
                        to<int_const>(while_block->cond)->value = 1;

                        // unconditional loops can have only one backedge !?
                        assert(loops[postorder]->loop_latch_blocks.size() == 1);
                        for (unsigned int i = ast_index + 2; i < loops[postorder]->loop_latch_blocks[0]->ast_index; i++) {
                            worklist.push_back(std::make_tuple(i, std::ref(if_else_block->stmts), nullptr));
                        }

                        worklist.push_back(std::make_tuple(ast_index, std::ref(if_else_block->stmts), to<stmt>(while_block)));
                    }
                    else if (if_else_block->stmts[ast_index] == loops[postorder]->header_block->parent) {
                        stmt_block::Ptr then_block = to<block::stmt_block>(to<block::if_stmt>(if_else_block->stmts[ast_index + 1])->then_stmt);
                        stmt_block::Ptr else_block = to<block::stmt_block>(to<block::if_stmt>(if_else_block->stmts[ast_index + 1])->else_stmt);

                        while_block->cond = to<block::if_stmt>(if_else_block->stmts[ast_index + 1])->cond;
                        if (then_block->stmts.size() == 0 && else_block->stmts.size() != 0) {
                            not_expr::Ptr new_cond = std::make_shared<not_expr>();
				            new_cond->static_offset = while_block->cond->static_offset;
				            new_cond->expr1 = while_block->cond;
				            while_block->cond = new_cond;

                            for (auto body_stmt: else_block->stmts) {
                                to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                            }

                            auto backedge_iter = std::find(backedge_blocks.begin(), backedge_blocks.end(), else_block);
                            if (backedge_iter != backedge_blocks.end()) {
                                std::cerr << "replaced BE\n";
                                std::replace(backedge_blocks.begin(), backedge_blocks.end(), *backedge_iter, to<stmt_block>(while_block->body));
                            }
                            while_block->continue_blocks.insert(while_block->continue_blocks.begin(), backedge_blocks.begin(), backedge_blocks.end());
                        }
                        else if (then_block->stmts.size() <= 2 && isa<block::break_stmt>(then_block->stmts.back())) {
                            not_expr::Ptr new_cond = std::make_shared<not_expr>();
                            new_cond->static_offset = while_block->cond->static_offset;
                            new_cond->expr1 = while_block->cond;
                            while_block->cond = new_cond;

                            for (auto body_stmt: else_block->stmts) {
                                to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                            }

                            then_block->stmts.pop_back();
                            for (auto stmt: then_block->stmts)
                                if_else_block->stmts.push_back(stmt);

                            auto backedge_iter = std::find(backedge_blocks.begin(), backedge_blocks.end(), else_block);
                            if (backedge_iter != backedge_blocks.end()) {
                                std::cerr << "replaced BE\n";
                                std::replace(backedge_blocks.begin(), backedge_blocks.end(), *backedge_iter, to<stmt_block>(while_block->body));
                            }
                            while_block->continue_blocks.insert(while_block->continue_blocks.begin(), backedge_blocks.begin(), backedge_blocks.end());
                        }
                        // else if (then_block->stmts.size() <= 2 && isa<block::break_stmt>(then_block->stmts.back())) {
                        //     not_expr::Ptr new_cond = std::make_shared<not_expr>();
                        //     new_cond->static_offset = while_block->cond->static_offset;
                        //     new_cond->expr1 = while_block->cond;
                        //     while_block->cond = new_cond;

                        //     then_block->stmts.pop_back();
                        //     for (auto stmt: then_block->stmts)
                        //         if_else_block->stmts.push_back(stmt);
                        // }
                        else {
                            for (auto body_stmt: then_block->stmts) {
                                to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                            }

                            auto backedge_iter = std::find(backedge_blocks.begin(), backedge_blocks.end(), then_block);
                            if (backedge_iter != backedge_blocks.end()) {
                                std::cerr << "replaced BE\n";
                                std::replace(backedge_blocks.begin(), backedge_blocks.end(), *backedge_iter, to<stmt_block>(while_block->body));
                            }
                            while_block->continue_blocks.insert(while_block->continue_blocks.begin(), backedge_blocks.begin(), backedge_blocks.end());
                        }
                        // for (auto body_stmt: else_block->stmts) {
                        //     to<stmt_block>(while_block->body)->stmts.push_back(body_stmt);
                        // }
                        // if block to be replaced with while block
                        worklist.push_back(std::make_tuple(ast_index, std::ref(if_else_block->stmts), to<stmt>(while_block)));
                    }
                }
            }

            // process worklist
            std::sort(worklist.begin(), worklist.end(), [](std::tuple<unsigned int, std::reference_wrapper<std::vector<stmt::Ptr>>, stmt::Ptr> a, std::tuple<unsigned int, std::reference_wrapper<std::vector<stmt::Ptr>>, stmt::Ptr> b) {
                return std::get<0>(a) > std::get<0>(b);
            });
            for (auto item : worklist) {
                std::vector<stmt::Ptr> &temp_ast = std::get<1>(item);
                if (std::get<2>(item)) {
                    std::replace(temp_ast.begin(), temp_ast.end(), temp_ast[std::get<0>(item) + 1], std::get<2>(item));
                    temp_ast.erase(temp_ast.begin() + std::get<0>(item));
                }
            }

            for (auto item : worklist) {
                std::vector<stmt::Ptr> &temp_ast = std::get<1>(item);
                if (!std::get<2>(item)) {
                    temp_ast.erase(temp_ast.begin() + std::get<0>(item));
                }
            }
            worklist.clear();

            std::cerr << "after ast\n";
            ast->dump(std::cerr, 0);
            std::cerr << "after ast\n";
        }
    }

    return ast;
}

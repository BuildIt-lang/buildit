#include "blocks/loops.h"
#include <algorithm>

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

    // Assign id to the loops
    for (unsigned int i = 0; i < loops.size(); i++) {
        loops[i]->loop_id = i;
    }

    // build a loop tree
    std::vector<bool> visited_loops(loops.size());
    visited_loops.assign(visited_loops.size(), false);
    for (auto loop: top_level_loops) {
        std::vector<int> postorder_loop_tree;
        visited_loops[loop->loop_id] = true;

        postorder_dfs_helper(postorder_loop_tree, visited_loops, loop->loop_id);
        postorder_loop_tree.push_back(loop->loop_id);
        postorder_loops_map[loop->loop_id] = postorder_loop_tree;
    }
}

static stmt::Ptr get_loop_block(std::shared_ptr<basic_block> loop_header, block::stmt_block::Ptr ast) {
    block::stmt::Ptr current_ast = to<block::stmt>(ast);
    std::vector<stmt::Ptr> current_block = to<block::stmt_block>(current_ast)->stmts;
    // unsigned int ast_index = loop_header->ast_index;
    std::deque<stmt::Ptr> worklist;
    std::map<stmt::Ptr, stmt::Ptr> ast_parent_map;

    for (auto stmt: current_block) {
        ast_parent_map[stmt] = current_ast;
    }
    worklist.insert(worklist.end(), current_block.begin(), current_block.end());

    while (worklist.size()) {
        stmt::Ptr worklist_top = worklist.front();
        worklist.pop_front();

        if (isa<block::stmt_block>(worklist_top)) {
            stmt_block::Ptr wl_stmt_block = to<stmt_block>(worklist_top);
            for (auto stmt: wl_stmt_block->stmts) {
                ast_parent_map[stmt] = worklist_top;
            }
            worklist.insert(worklist.end(), wl_stmt_block->stmts.begin(), wl_stmt_block->stmts.end());
        }
        else if (isa<block::if_stmt>(worklist_top)) {
            if_stmt::Ptr wl_if_stmt = to<if_stmt>(worklist_top);

            if (to<stmt_block>(wl_if_stmt->then_stmt)->stmts.size() != 0) {
                stmt_block::Ptr wl_if_then_stmt = to<stmt_block>(wl_if_stmt->then_stmt);
                for (auto stmt: wl_if_then_stmt->stmts) {
                    ast_parent_map[stmt] = worklist_top;
                }
                worklist.insert(worklist.end(), wl_if_then_stmt->stmts.begin(), wl_if_then_stmt->stmts.end());
            }
            if (to<stmt_block>(wl_if_stmt->else_stmt)->stmts.size() != 0) {
                stmt_block::Ptr wl_if_else_stmt = to<stmt_block>(wl_if_stmt->else_stmt);
                for (auto stmt: wl_if_else_stmt->stmts) {
                    ast_parent_map[stmt] = worklist_top;
                }
                worklist.insert(worklist.end(), wl_if_else_stmt->stmts.begin(), wl_if_else_stmt->stmts.end());
            }
        }
        else if (isa<block::label_stmt>(worklist_top)) {
            label_stmt::Ptr wl_label_stmt = to<label_stmt>(worklist_top);
            if (worklist_top == loop_header->parent)
                return ast_parent_map[worklist_top];
        }
        else if (isa<block::goto_stmt>(worklist_top)) {
            goto_stmt::Ptr wl_goto_stmt = to<goto_stmt>(worklist_top);
            if (worklist_top == loop_header->parent)
                return ast_parent_map[worklist_top];
        }
    }

    return nullptr;
}

static void replace_loop_latches(std::shared_ptr<loop> loop, block::stmt_block::Ptr ast) {
    for (auto latch : loop->loop_latch_blocks) {
        stmt::Ptr loop_latch_ast = get_loop_block(latch, ast);
        if (isa<stmt_block>(loop_latch_ast)) {
            std::vector<stmt::Ptr> &temp_loop_ast = to<stmt_block>(loop_latch_ast)->stmts;
            std::replace(temp_loop_ast.begin(), temp_loop_ast.end(), temp_loop_ast[latch->ast_index], to<stmt>(std::make_shared<continue_stmt>()));
        }
        else if (isa<if_stmt>(loop_latch_ast)) {
            stmt_block::Ptr if_then_block = to<block::stmt_block>(to<block::if_stmt>(loop_latch_ast)->then_stmt);
            stmt_block::Ptr if_else_block = to<block::stmt_block>(to<block::if_stmt>(loop_latch_ast)->else_stmt);

            if (if_then_block->stmts.size() && if_then_block->stmts[latch->ast_index] == latch->parent) {
                std::replace(if_then_block->stmts.begin(), if_then_block->stmts.end(), if_then_block->stmts[latch->ast_index], to<stmt>(std::make_shared<continue_stmt>()));
            }
            else if (if_else_block->stmts.size() && if_else_block->stmts[latch->ast_index] == latch->parent) {
                std::replace(if_else_block->stmts.begin(), if_else_block->stmts.end(), if_else_block->stmts[latch->ast_index], to<stmt>(std::make_shared<continue_stmt>()));
            }
        }
    }
}

block::stmt_block::Ptr loop_info::convert_to_ast(block::stmt_block::Ptr ast) {
    for (auto loop_map: postorder_loops_map) {
        // std::cerr << "== top level loop tree ==\n";
        for (auto postorder: loop_map.second) {
            // std::cerr << postorder <<"\n";
            block::stmt::Ptr loop_header_ast = get_loop_block(loops[postorder]->header_block, ast);

            while_stmt::Ptr while_block = std::make_shared<while_stmt>();
            while_block->cond = std::make_shared<int_const>();
            to<int_const>(while_block->cond)->value = 1;
            while_block->body = std::make_shared<stmt_block>();

            if (isa<block::stmt_block>(loop_header_ast)) {
                unsigned int ast_index = loops[postorder]->header_block->ast_index;
                if (to<block::stmt_block>(loop_header_ast)->stmts[ast_index] == loops[postorder]->header_block->parent) {
                    stmt_block::Ptr then_block = to<block::stmt_block>(to<block::if_stmt>(to<block::stmt_block>(loop_header_ast)->stmts[ast_index + 1])->then_stmt);
                    stmt_block::Ptr else_block = to<block::stmt_block>(to<block::if_stmt>(to<block::stmt_block>(loop_header_ast)->stmts[ast_index + 1])->else_stmt);

                    // if (isa<goto_stmt>(then_block->stmts.back())) {
                    //     then_block->stmts.pop_back();
                    //     then_block->stmts.push_back(std::make_shared<continue_stmt>());
                    // }
                    replace_loop_latches(loops[postorder], ast);

                    else_block->stmts.push_back(std::make_shared<break_stmt>());
                    to<stmt_block>(while_block->body)->stmts.push_back(to<block::stmt_block>(loop_header_ast)->stmts[ast_index + 1]);
                    // while_block->cond = to<block::if_stmt>(to<block::stmt_block>(loop_header_ast)->stmts[ast_index + 1])->cond;
                    // while_block->dump(std::cerr, 0);
                    // std::cerr << "found loop header in stmt block\n";

                    // if block to be replaced with while block
                    std::vector<stmt::Ptr> &temp_ast = to<block::stmt_block>(loop_header_ast)->stmts;
                    std::replace(temp_ast.begin(), temp_ast.end(), temp_ast[ast_index + 1], to<stmt>(while_block));
                    temp_ast.erase(temp_ast.begin() + ast_index);
                }
                else {
                    // std::cerr << "not found loop header in stmt block\n";
                }
            }
            else if (isa<block::if_stmt>(loop_header_ast)) {
                unsigned int ast_index = loops[postorder]->header_block->ast_index;
                stmt_block::Ptr if_then_block = to<block::stmt_block>(to<block::if_stmt>(loop_header_ast)->then_stmt);
                stmt_block::Ptr if_else_block = to<block::stmt_block>(to<block::if_stmt>(loop_header_ast)->else_stmt);

                if (if_then_block->stmts.size() != 0) {
                    if (if_then_block->stmts[ast_index] == loops[postorder]->header_block->parent) {
                        stmt_block::Ptr then_block = to<block::stmt_block>(to<block::if_stmt>(if_then_block->stmts[ast_index + 1])->then_stmt);
                        stmt_block::Ptr else_block = to<block::stmt_block>(to<block::if_stmt>(if_then_block->stmts[ast_index + 1])->else_stmt);

                        replace_loop_latches(loops[postorder], ast);

                        else_block->stmts.push_back(std::make_shared<break_stmt>());
                        to<stmt_block>(while_block->body)->stmts.push_back(if_then_block->stmts[ast_index + 1]);
                        // while_block->cond = to<block::if_stmt>(loop_header_ast)->cond;

                        // while_block->dump(std::cerr, 0);
                        // std::cerr << "found loop header in if-then stmt\n";

                        // if block to be replaced with while block
                        std::vector<stmt::Ptr> &temp_ast = if_then_block->stmts;
                        std::replace(temp_ast.begin(), temp_ast.end(), temp_ast[ast_index + 1], to<stmt>(while_block));
                        temp_ast.erase(temp_ast.begin() + ast_index);
                    }
                    else {
                        // loop_header_ast->dump(std::cerr, 0);
                        // std::cerr << "not found loop header in if-then stmt\n";
                    }
                }
                else if (if_else_block->stmts.size() != 0) {
                    if (if_else_block->stmts[ast_index] == loops[postorder]->header_block->parent) {
                        stmt_block::Ptr then_block = to<block::stmt_block>(to<block::if_stmt>(if_else_block->stmts[ast_index + 1])->then_stmt);
                        stmt_block::Ptr else_block = to<block::stmt_block>(to<block::if_stmt>(if_else_block->stmts[ast_index + 1])->else_stmt);

                        replace_loop_latches(loops[postorder], ast);

                        else_block->stmts.push_back(std::make_shared<break_stmt>());
                        to<stmt_block>(while_block->body)->stmts.push_back(if_else_block->stmts[ast_index + 1]);
                        // while_block->cond = to<block::if_stmt>(loop_header_ast)->cond;

                        // while_block->dump(std::cerr, 0);
                        // std::cerr << "found loop header in if-else stmt\n";

                        // if block to be replaced with while block
                        std::vector<stmt::Ptr> &temp_ast = if_else_block->stmts;
                        std::replace(temp_ast.begin(), temp_ast.end(), temp_ast[ast_index + 1], to<stmt>(while_block));
                        temp_ast.erase(temp_ast.begin() + ast_index);
                    }
                    else {
                        // loop_header_ast->dump(std::cerr, 0);
                        // std::cerr << "not found loop header in if-else stmt\n";
                    }
                }
            }
            else {
                // std::cerr << "loop header not found\n";
            }
            // insert into AST - std::replace
            // set the ast to loop depth + 1
            // loops[loop_tree.first]->header_block->ast_index
        }
    }

    return ast;
}

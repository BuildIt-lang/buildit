#include "blocks/loops.h"
#include <unordered_set>
#include <algorithm>
#include <tuple>
#include <set>

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
        if (unique_postdom == -1)
            continue;

        bool unique_postdom_flag = true;
        for (auto exit_bb: loop->loop_exit_blocks) {
            if (post_dta.get_idom(exit_bb->id) != unique_postdom) {
                unique_postdom_flag = false;
            }
        }

        if (unique_postdom_flag)
            loop->unique_exit_block = dta.cfg_[unique_postdom];
    }

    // Populate loop condition block
    for(auto loop: loops) {
        if (!loop->header_block)
            continue;

        // this might be an unconditional loop or
        // infinite loop.
        if (loop->loop_exit_blocks.empty())
            continue;

        std::shared_ptr<basic_block> header = loop->header_block;
        assert(header->successor.size() == 1 && "loop header cannot have more than one successor");
        if (isa<if_stmt>(header->successor[0]->parent))
            loop->condition_block = header->successor[0];
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

std::set<stmt::Ptr> visited_blocks;
std::map<stmt_block::Ptr, stmt_block::Ptr> ast_parent_map_loop;
// std::map<std::shared_ptr<basic_block>, while_stmt::Ptr> return_blocks_parent_loop;
int jump_condition_counter = 0;
stmt::Ptr loop::convert_to_ast_impl(loop_info &li, dominator_analysis &dta_, std::vector<std::pair<std::shared_ptr<basic_block>, stmt_block::Ptr>> &return_blocks, stmt::Ptr &jump_condition_def, stmt::Ptr &jump_condition_block) {
    while_stmt::Ptr while_block = std::make_shared<while_stmt>();
    while_block->body = std::make_shared<stmt_block>();
    structured_ast_loop = while_block;

    if (!condition_block) {
        while_block->cond = std::make_shared<int_const>();
	    to<int_const>(while_block->cond)->value = 1;
    }
    std::deque<std::pair<std::shared_ptr<basic_block>, stmt_block::Ptr>> worklist;
    std::unordered_set<std::shared_ptr<basic_block>> visited;
    worklist.push_back({header_block->successor[0], to<stmt_block>(while_block->body)});
    visited.insert(header_block->successor[0]);
    
    std::cerr << "loop: " << loop_id << " " << header_block->id << "\n";
    while (worklist.size()) {
        auto bb_ast_pair = worklist.front();
        auto bb = bb_ast_pair.first;
        auto ast = bb_ast_pair.second;
        worklist.pop_front();
        
        if (isa<label_stmt>(bb->parent)) {
            for (auto subloop: subloops) {
                if (subloop->header_block->parent == bb->parent) {
                    std::cerr << "found subloop\n";
                    std::vector<std::pair<std::shared_ptr<basic_block>, stmt_block::Ptr>> loop_out_blocks;
                    stmt::Ptr jump_def, jump_block;
                    ast->stmts.push_back(subloop->convert_to_ast_impl(li, dta_, loop_out_blocks, jump_def, jump_block));

                    for (auto block: loop_out_blocks) {
                        // return_blocks_parent_loop.insert({block.first, to<while_stmt>(ast->stmts.back())});
                        worklist.push_back({block.first, block.second ? block.second : ast});
                    }
                    std::cerr << "finish subloop\n";

                    break;
                }
            }
        }
        else if (isa<if_stmt>(bb->parent)) {
            if (!blocks_id_map.count(bb->id))
                continue;

            if_stmt::Ptr if_stmt_copy = std::make_shared<if_stmt>();
            if_stmt_copy->then_stmt = to<stmt>(std::make_shared<stmt_block>());
            if_stmt_copy->else_stmt = to<stmt>(std::make_shared<stmt_block>());
            if_stmt_copy->annotation = to<if_stmt>(bb->parent)->annotation;

            if (condition_block == bb) {
                while_block->cond = to<if_stmt>(bb->parent)->cond;
                
                if (to<stmt_block>(to<if_stmt>(bb->parent)->then_stmt)->stmts.size() == 0) {
                    std::cerr << "negated if cond\n";
                    not_expr::Ptr negated_cond = std::make_shared<not_expr>();
				    negated_cond->static_offset = while_block->cond->static_offset;
				    negated_cond->expr1 = while_block->cond;
                    while_block->cond = negated_cond;

                    if (bb->else_branch) {
                        ast_parent_map_loop[to<stmt_block>(while_block->body)] = ast;
                        worklist.push_back({bb->else_branch, to<stmt_block>(while_block->body)});
                        visited.insert(bb->else_branch);
                    }

                    if (!blocks_id_map.count(bb->successor[1]->id) && blocks_id_map.count(bb->successor[0]->id)) {
                        std::cerr << "inserting out of loop block (1): " << bb->successor[0]->id << bb->successor[1]->is_exit_block << "\n";
                        ast_parent_map_loop[to<stmt_block>(while_block->body)] = ast;
                        worklist.push_back({bb->successor[1], nullptr});
                        visited.insert(bb->successor[1]);
                    }
                    else if (blocks_id_map.count(bb->successor[1]->id) && !blocks_id_map.count(bb->successor[0]->id)){
                        std::cerr << "inserting out of loop block (0): " << bb->successor[0]->id << bb->successor[0]->is_exit_block << "\n";
                        worklist.push_back({bb->successor[0], nullptr});
                        visited.insert(bb->successor[0]);
                    }
                }
                else {
                    if (bb->then_branch && blocks_id_map.count(bb->then_branch->id)) {
                        std::cerr << "loop cond then branch: " << bb->then_branch->id << "\n";
                        ast_parent_map_loop[to<stmt_block>(while_block->body)] = ast;
                        worklist.push_back({bb->then_branch, to<stmt_block>(while_block->body)});
                        visited.insert(bb->then_branch);

                        // if (!blocks_id_map.count(bb->successor[1]->id)) {
                        //     std::cerr << "pushing out of loop branch (1) out: " << bb->successor[1]->id << "\n";
                        //     worklist.push_back({bb->successor[1], ast_parent_map_loop[to<stmt_block>(while_block->body)]});
                        //     visited.insert(bb->successor[1]);                        
                        // }
                    }

                    if (bb->else_branch && blocks_id_map.count(bb->else_branch->id)) {
                        not_expr::Ptr negated_cond = std::make_shared<not_expr>();
                        negated_cond->static_offset = while_block->cond->static_offset;
                        negated_cond->expr1 = while_block->cond;
                        while_block->cond = negated_cond;

                        std::cerr << "loop cond else branch: " << bb->else_branch->id << "\n";
                        ast_parent_map_loop[to<stmt_block>(while_block->body)] = ast;
                        worklist.push_back({bb->else_branch, to<stmt_block>(while_block->body)});
                        visited.insert(bb->else_branch);

                        // if (!blocks_id_map.count(bb->successor[0]->id)) {
                        //     std::cerr << "pushing out of loop branch (0) out: " << bb->successor[0]->id << "\n";
                        //     worklist.push_back({bb->successor[0], ast_parent_map_loop[to<stmt_block>(while_block->body)]});
                        //     visited.insert(bb->successor[0]);                        
                        // }
                    }
                    
                    if (blocks_id_map.count(bb->successor[1]->id) && !blocks_id_map.count(bb->successor[0]->id)) {
                        std::cerr << "inserting out of loop block (0): " << bb->successor[0]->id << bb->successor[0]->is_exit_block << "\n";
                        ast_parent_map_loop[to<stmt_block>(while_block->body)] = ast;
                        worklist.push_back({bb->successor[0], ast_parent_map_loop[to<stmt_block>(while_block->body)]});
                        visited.insert(bb->successor[0]);
                    }
                    else if (!blocks_id_map.count(bb->successor[1]->id) && blocks_id_map.count(bb->successor[0]->id)) {
                        std::cerr << "inserting out of loop block (1): " << bb->successor[1]->id << bb->successor[1]->is_exit_block << "\n";
                        worklist.push_back({bb->successor[1], nullptr});
                        visited.insert(bb->successor[1]);
                    }
                }
            }
            else {
                if_stmt_copy->cond = to<if_stmt>(bb->parent)->cond;
                bb->parent->dump(std::cerr, 0);
                std::cerr << bb->successor[0]->id << " " << bb->successor[1]->id << "\n";

                if (bb->then_branch) {
                    std::cerr << "non-cond if then: " << bb->id << "\n";
                    ast_parent_map_loop[to<stmt_block>(if_stmt_copy->then_stmt)] = ast;
                    if (!blocks_id_map.count(bb->then_branch->id) && !bb->then_branch->is_exit_block) {
                        return_blocks.push_back({bb->then_branch, to<stmt_block>(if_stmt_copy->then_stmt)});
                    }
                    else {
                        worklist.push_back({bb->then_branch, to<stmt_block>(if_stmt_copy->then_stmt)});
                    }
                    visited.insert(bb->then_branch);
                }

                if (bb->else_branch) {
                    std::cerr << "non-cond if else: " << bb->id <<"\n";
                    ast_parent_map_loop[to<stmt_block>(if_stmt_copy->else_stmt)] = ast;
                    if (!blocks_id_map.count(bb->else_branch->id) && !bb->else_branch->is_exit_block) {
                        return_blocks.push_back({bb->else_branch, to<stmt_block>(if_stmt_copy->else_stmt)});
                    }
                    else {
                        worklist.push_back({bb->else_branch, to<stmt_block>(if_stmt_copy->else_stmt)});
                    }
                    visited.insert(bb->else_branch);
                }

                if ((!bb->then_branch || !bb->else_branch) && bb->successor.size() == 2 && bb->successor[1]->is_exit_block) {
                    std::cerr << "inserting out of loop block" << bb->successor[1]->id << bb->successor[1]->is_exit_block << "\n";
                    worklist.push_back({bb->successor[1], nullptr});
                    visited.insert(bb->successor[1]);
                }

                ast->stmts.push_back(to<stmt>(if_stmt_copy));
            }
        }
        else if (isa<goto_stmt>(bb->parent)) {
            if (to<goto_stmt>(bb->parent)->label1 == to<label_stmt>(header_block->parent)->label1) {
                bool is_last_block = false;
                bool is_goto_to_outerloop = false;
                bb->parent->dump(std::cerr, 0);
                if (dta_.get_preorder_bb_map()[bb->id] == (int)dta_.get_preorder().size() - 1) {
                    is_last_block = true;
                }
                else {
                    // TODO: this can be cleaned up or reduced.
                    int next_preorder = dta_.get_preorder_bb_map()[bb->id] + 1 < (int)dta_.get_preorder().size() ? dta_.get_preorder()[dta_.get_preorder_bb_map()[bb->id] + 1] : -1;
                    int next_next_preorder = dta_.get_preorder_bb_map()[next_preorder] + 1 < (int)dta_.get_preorder().size() ? dta_.get_preorder()[dta_.get_preorder_bb_map()[next_preorder] + 1] : -1;

                    if (loop_latch_blocks.size() == 1)
                        is_last_block = true;
                    else if (blocks_id_map.count(next_preorder))
                        is_last_block = false;
                    else {
                        if (!blocks_id_map.count(next_preorder) || !blocks_id_map.count(next_next_preorder))
                            is_last_block = true;
                        else if (unique_exit_block && (next_preorder == (int)unique_exit_block->id))
                            is_last_block = true;
                        else if (unique_exit_block && (next_next_preorder == (int)unique_exit_block->id))
                            is_last_block = true;
                        else if (next_preorder != -1 && next_next_preorder == -1 && dta_.cfg_[next_preorder]->is_exit_block)
                            is_last_block = true;
                        else if (next_preorder != -1 && next_next_preorder != -1 && dta_.cfg_[next_preorder]->is_exit_block && isa<goto_stmt>(dta_.cfg_[next_next_preorder]->parent))
                            is_last_block = true;
                        else
                            is_last_block = false;
                    }
                }

                if (!is_last_block && (int)bb->ast_depth - 2 > (int)header_block->ast_depth) {
                    // figure out the parent while loop for this basic block
                    auto target_bb = bb->successor[0];
                    auto pointer_bb = bb;
                    std::vector<std::shared_ptr<loop>> loop_parent_tree;
                    do {
                        std::cerr << "iter: " << pointer_bb->id << "\n";
                        
                        if (pointer_bb->predecessor.size() > 1) {
                            auto min_preorder_pred = std::min_element(pointer_bb->predecessor.begin(), pointer_bb->predecessor.end(),
                            [&dta_](std::shared_ptr<basic_block> bb1, std::shared_ptr<basic_block> bb2) {
                                return dta_.get_preorder_bb_map()[bb1->id] < dta_.get_preorder_bb_map()[bb2->id];
                            });
                            if (min_preorder_pred != pointer_bb->predecessor.end()) {
                                pointer_bb = *min_preorder_pred;
                            }
                        }
                        else {
                            pointer_bb = pointer_bb->predecessor[0];
                        }

                        for (auto loop: li.loops) {
                            if (loop->header_block == pointer_bb) {
                                loop_parent_tree.push_back(loop);
                                break;
                            }
                        }
                    } while (target_bb != pointer_bb);
                    
                    for (auto loops : loop_parent_tree) {
                        std::cerr << loops->header_block->id << "\n";
                    }

                    while_stmt::Ptr jump_target_loop = loop_parent_tree.back()->structured_ast_loop;
                    while_stmt::Ptr guard_target_loop = (*(loop_parent_tree.rbegin() + 1))->structured_ast_loop;

                    std::cerr << "handingling break cond\n";
                    is_goto_to_outerloop = true;

                    auto jump_cond_def = std::make_shared<var>();
                    jump_cond_def->var_name = "control_guard" + std::to_string(jump_condition_counter++);

                    auto scalar_type1 = std::make_shared<scalar_type>();
                    jump_cond_def->var_type = scalar_type1;
                    scalar_type1->scalar_type_id = scalar_type::INT_TYPE;

                    auto var_expr1 = std::make_shared<var_expr>();
                    var_expr1->var1 = jump_cond_def;

                    auto const_expr1 = std::make_shared<int_const>();
                    const_expr1->value = 1;
                    const_expr1->is_64bit = false;

                    auto assign_expr1 = std::make_shared<assign_expr>();
                    assign_expr1->var1 = var_expr1;
                    assign_expr1->expr1 = const_expr1;

                    auto jump_expr = std::make_shared<expr_stmt>();
                    jump_expr->expr1 = assign_expr1;

                    ast->stmts.push_back(jump_expr);

                    auto var_expr2 = std::make_shared<var_expr>();
                    var_expr2->var1 = jump_cond_def;

                    auto const_expr2 = std::make_shared<int_const>();
                    const_expr2->value = 0;
                    const_expr2->is_64bit = false;

                    auto assign_expr2 = std::make_shared<assign_expr>();
                    assign_expr2->var1 = var_expr2;
                    assign_expr2->expr1 = const_expr2;

                    auto expr_stmt2 = std::make_shared<expr_stmt>();
                    expr_stmt2->expr1 = assign_expr2;

                    // guard decl stmt
                    auto var_decl1 = std::make_shared<decl_stmt>();
                    var_decl1->decl_var = jump_cond_def;
                    var_decl1->init_expr = const_expr2;

                    auto while_body = to<stmt_block>(while_block->body);
                    while_body->stmts.insert(while_body->stmts.begin(), var_decl1);

                    // guard if stmt
                    auto if_stmt1 = std::make_shared<if_stmt>();
                    if_stmt1->else_stmt = std::make_shared<stmt_block>();
                    auto stmt_block1 = std::make_shared<stmt_block>();
                    if_stmt1->then_stmt = stmt_block1;
                    stmt_block1->stmts.push_back(std::make_shared<continue_stmt>());

                    auto var_expr3 = std::make_shared<var_expr>();
                    var_expr3->var1 = jump_cond_def;
                    if_stmt1->cond = var_expr3;

                    auto guard_while_body = to<stmt_block>(guard_target_loop->body);
                    auto jump_while_body = to<stmt_block>(jump_target_loop->body);

                    guard_while_body->stmts.insert(guard_while_body->stmts.begin(), to<stmt>(expr_stmt2));

                    auto guard_decl_insertion_point = std::find(jump_while_body->stmts.begin(), jump_while_body->stmts.end(), to<stmt>(guard_target_loop));
                    if (guard_decl_insertion_point != jump_while_body->stmts.end()) {
                        jump_while_body->stmts.insert(guard_decl_insertion_point + 1, to<stmt>(if_stmt1));
                    }
                }
                if (!is_last_block) {
                    std::cerr << "inserted continue: " << bb->id << loop_id << "\n";
                    ast->stmts.push_back(is_goto_to_outerloop ? to<stmt>(std::make_shared<break_stmt>()) : to<stmt>(std::make_shared<continue_stmt>()));
                    while_block->continue_blocks.push_back(ast);
                }
                visited.insert(bb);
            }
        }
        else {
            assert(bb->successor.size() <= 1);
            bool exit_bb_succ = false;

            if (bb->is_exit_block && !blocks_id_map.count(bb->id)) {
                for (auto subloop: subloops) {
                    if (bb == subloop->unique_exit_block) {
                        ast->stmts.push_back(to<stmt>(std::make_shared<break_stmt>()));
                    }
                }
                for (auto exit: loop_exit_blocks) {
                    for (auto pred: bb->predecessor) {
                        if (pred->id == exit->id && bb->successor.size()) {
                            return_blocks.push_back({bb->successor[0], ast});
                            std::cerr << "found return block " << bb->successor[0]->id << "\n";
                            exit_bb_succ = true;
                            break;
                        }
                    }

                    if (exit_bb_succ)
                        break;
                }

                if (exit_bb_succ)
                    continue;
                
                if (bb == unique_exit_block && dta_.get_preorder_bb_map()[bb->id] != (int)dta_.get_preorder().size() - 1) {
                    ast->stmts.push_back(to<stmt>(std::make_shared<break_stmt>()));
                }
            }

            if (!blocks_id_map.count(bb->id) && !bb->is_exit_block) {
                std::cerr << "case for 26: " << bb->id << worklist.size() << loop_id << "\n";
                return_blocks.push_back({bb, nullptr});
                continue;
            }

            if (!bb->is_exit_block && !isa<stmt_block>(bb->parent)) {
                std::cerr << "bb: " << bb->id << "\n";
                ast->dump(std::cerr, 0);
                ast->stmts.push_back(to<stmt>(bb->parent));
            }
            
            if (bb->successor.size()) {
                if (visited.count(bb->successor[0]))
                    continue;
                else
                    visited.insert(bb->successor[0]);
    
                if (bb->is_exit_block) {
                    worklist.push_back({bb->successor[0], ast_parent_map_loop[ast]});
                }
                else {
                    worklist.push_back({bb->successor[0], ast});
                }
            }
        }
    }

    return to<stmt>(while_block);
}

std::map<stmt_block::Ptr, stmt_block::Ptr> ast_parent_map_global;
block::stmt_block::Ptr loop_info::convert_to_ast(block::stmt_block::Ptr ast) {
    block::stmt_block::Ptr return_ast = std::make_shared<stmt_block>();

    std::deque<std::pair<std::shared_ptr<basic_block>, stmt_block::Ptr>> worklist;
    std::unordered_set<std::shared_ptr<basic_block>> visited;
    worklist.push_back({dta.cfg_[0], return_ast});
    visited.insert(dta.cfg_[0]);

    while (worklist.size()) {
        auto bb_ast_pair = worklist.front();
        auto bb = bb_ast_pair.first;
        auto ast = bb_ast_pair.second;
        worklist.pop_front();

        if (isa<label_stmt>(bb->parent)) {
            for (auto loop : top_level_loops) {
                if (loop->header_block->parent == bb->parent) {
                    std::cerr << "found outerloop\n";
                    stmt::Ptr jump_def, jump_block;
                    std::vector<std::pair<std::shared_ptr<basic_block>, stmt_block::Ptr>> loop_out_blocks;
                    ast->stmts.push_back(loop->convert_to_ast_impl(*this, dta, loop_out_blocks, jump_def, jump_block));
                    loop->structured_ast_loop = to<while_stmt>(ast->stmts.back());

                    for (auto block: loop_out_blocks) {
                        worklist.push_back({block.first, block.second ? block.second : ast});
                    }
                    std::cerr << "finish outerloop\n";
                    std::cerr << worklist.size() << "\n";
                    break;
                }
            }
        }
        else if (isa<if_stmt>(bb->parent)) {

            if (bb_loop_map.count(bb->id))
                continue;
            
            if_stmt::Ptr if_stmt_copy = std::make_shared<if_stmt>();
            if_stmt_copy->then_stmt = to<stmt>(std::make_shared<stmt_block>());
            if_stmt_copy->else_stmt = to<stmt>(std::make_shared<stmt_block>());
            if_stmt_copy->cond = to<if_stmt>(bb->parent)->cond;
            if_stmt_copy->annotation = to<if_stmt>(bb->parent)->annotation;

            // push the then branch onto worklist. (worklist should be a pair <processed, destination>) ?
            if (bb->then_branch) {
                ast_parent_map_global[to<stmt_block>(if_stmt_copy->then_stmt)] = ast;
                worklist.push_back({bb->then_branch, to<stmt_block>(if_stmt_copy->then_stmt)});
                visited.insert(bb->then_branch);
            }

            if (bb->else_branch) {
                ast_parent_map_global[to<stmt_block>(if_stmt_copy->else_stmt)] = ast;
                worklist.push_back({bb->else_branch, to<stmt_block>(if_stmt_copy->else_stmt)});
                visited.insert(bb->else_branch);
            }

            ast->stmts.push_back(to<stmt>(if_stmt_copy));
        }
        else {
            assert(bb->successor.size() <= 1);

            if (bb->is_exit_block) {
                for (auto loop: loops) {
                    if (bb == loop->unique_exit_block) {
                        std::cerr << "inserted break: " << bb->id << "\n";
                        ast->stmts.push_back(to<stmt>(std::make_shared<break_stmt>()));
                    }
                }
            }

            if (bb_loop_map.count(bb->id))
                continue;

            // what is happening is that when we see a if stmt exit block we should
            // reduce the level of the tree, it is not being done now.
            if (!bb->is_exit_block && !isa<stmt_block>(bb->parent)) {
                std::cerr << "out bb: " << bb->id << "\n";
                ast->stmts.push_back(to<stmt>(bb->parent));
            }
            
            if (bb->successor.size()) {
                if (visited.count(bb->successor[0]))
                    continue;
                else
                    visited.insert(bb->successor[0]);
    
                if (bb->is_exit_block)
                    worklist.push_back({bb->successor[0], ast_parent_map_global[ast]});
                else
                    worklist.push_back({bb->successor[0], ast});
            }
        }
    }
    
    return return_ast;
}

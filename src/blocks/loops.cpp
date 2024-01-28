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
stmt::Ptr loop::convert_to_ast_impl(loop_info &li, dominator_analysis &dta_, std::vector<std::pair<std::shared_ptr<basic_block>, stmt_block::Ptr>> &return_blocks) {
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
                    ast->stmts.push_back(subloop->convert_to_ast_impl(li, dta_, loop_out_blocks));

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
                    std::cerr << "non-cond if then: " << bb->id << " " << bb->then_branch->id << "\n";
                    bb->then_branch->parent->dump(std::cerr, 0);
                    to<stmt_block>(if_stmt_copy->then_stmt)->dump(std::cerr, 0);
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
                    std::cerr << "non-cond if else: " << bb->id << " " << bb->else_branch->id <<"\n";
                    bb->else_branch->parent->dump(std::cerr, 0);
                    to<stmt_block>(if_stmt_copy->else_stmt)->dump(std::cerr, 0);
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
                std::cerr << "goto handler\n";
                std::cerr << "ast dump\n";
                ast->dump(std::cerr, 0);
                std::cerr << "bb dump\n";
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
                    std::vector<std::shared_ptr<basic_block>> mixed_walkback_tree;
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

                        // runtime can be optimized if we create a hashmap for (header_block->id => loop) map
                        // also a map for (conditional_block->id => loop)
                        for (auto loop: li.loops) {
                            if (isa<if_stmt>(pointer_bb->parent) && mixed_walkback_tree.size() == 0) {
                                bool skip_if_stmt = false;
                                for (auto iloop: li.loops) {
                                    if (iloop->condition_block->id == pointer_bb->id) {
                                        skip_if_stmt = true;
                                        break;
                                    }
                                }
                                if (skip_if_stmt)
                                    continue;

                                std::cerr << "looping back from a if statement\n";
                                mixed_walkback_tree.push_back(pointer_bb);
                            }

                            if (loop->header_block == pointer_bb) {
                                loop_parent_tree.push_back(loop);
                                mixed_walkback_tree.push_back(loop->header_block);
                                break;
                            }
                        }
                    } while (target_bb != pointer_bb);
                    
                    std::cerr << "loop parent tree\n";
                    for (auto loops : loop_parent_tree) {
                        std::cerr << loops->header_block->id << "\n";
                    }

                    std::cerr << "mixed walkback tree\n";
                    for (auto bb: mixed_walkback_tree) {
                        std::cerr << bb->id << "\n";
                    }

                    bool use_mixed_tree = false;
                    if (isa<if_stmt>(mixed_walkback_tree[0]->parent) && mixed_walkback_tree.size() > loop_parent_tree.size())
                        use_mixed_tree = true;
                    
                    // generate guard variable definitions
                    std::cerr << "creating guard variable definitions\n";
                    std::vector<std::shared_ptr<var>> guard_variables;
                    std::vector<std::shared_ptr<decl_stmt>> guard_variable_defs;
                    for (unsigned i = 0; i < loop_parent_tree.size() - 2 + (unsigned)use_mixed_tree; i++) {
                        // create the var and set it's attributes
                        auto guard_variable = std::make_shared<var>();
                        guard_variable->var_name = "control_guard" + std::to_string(jump_condition_counter++);
                        guard_variable->var_type = std::make_shared<scalar_type>();
                        to<scalar_type>(guard_variable->var_type)->scalar_type_id = scalar_type::INT_TYPE;
                        
                        // create a constant equal to 0
                        auto int_constant_zero = std::make_shared<int_const>();
                        int_constant_zero->value = 0;
                        int_constant_zero->is_64bit = false;

                        // create the variable declaration statement
                        auto guard_variable_declaration = std::make_shared<decl_stmt>();
                        guard_variable_declaration->decl_var = guard_variable;
                        guard_variable_declaration->init_expr = int_constant_zero;

                        guard_variables.push_back(guard_variable);
                        guard_variable_defs.push_back(guard_variable_declaration);
                        guard_variable_declaration->dump(std::cerr, 0);
                    }

                    std::cerr << "creating guard variable assign statements\n";
                    std::vector<std::pair<std::shared_ptr<expr_stmt>, std::shared_ptr<expr_stmt>>> set_guard_variable_exprs;
                    for (auto guard_var: guard_variables) {
                        // create a constant equal to 0
                        auto int_constant_zero = std::make_shared<int_const>();
                        int_constant_zero->value = 0;
                        int_constant_zero->is_64bit = false;

                        // create a constant equal to 1
                        auto int_constant_one = std::make_shared<int_const>();
                        int_constant_one->value = 1;
                        int_constant_one->is_64bit = false;

                        // create an expr for guard variable equal to 0
                        auto guard_variable_expr_zero = std::make_shared<var_expr>();
                        guard_variable_expr_zero->var1 = guard_var;

                        // create an expr for guard variable equal to 1
                        auto guard_variable_expr_one = std::make_shared<var_expr>();
                        guard_variable_expr_one->var1 = guard_var;

                        // create a guard variable assign expr for equal to 0
                        auto guard_variable_assign_expr_zero = std::make_shared<assign_expr>();
                        guard_variable_assign_expr_zero->var1 = guard_variable_expr_zero;
                        guard_variable_assign_expr_zero->expr1 = int_constant_zero;

                        // create a guard variable assign expr for equal to 1
                        auto guard_variable_assign_expr_one = std::make_shared<assign_expr>();
                        guard_variable_assign_expr_one->var1 = guard_variable_expr_one;
                        guard_variable_assign_expr_one->expr1 = int_constant_one;

                        // create the guard variable expression statement equal to 0
                        auto guard_variable_expr_statement_zero = std::make_shared<expr_stmt>();
                        guard_variable_expr_statement_zero->expr1 = guard_variable_assign_expr_zero;

                        // create the guard variable expression statement equal to 1
                        auto guard_variable_expr_statement_one = std::make_shared<expr_stmt>();
                        guard_variable_expr_statement_one->expr1 = guard_variable_assign_expr_one;

                        set_guard_variable_exprs.push_back({guard_variable_expr_statement_zero, guard_variable_expr_statement_one});
                        guard_variable_expr_statement_zero->dump(std::cerr, 0);
                        guard_variable_expr_statement_one->dump(std::cerr, 0);
                    }

                    std::cerr << "creating guard if statements\n";
                    std::vector<std::shared_ptr<if_stmt>> guard_if_blocks;
                    for (unsigned i = 0; i < guard_variables.size(); i++) {
                        // create guard if statement
                        auto guard_if_statement = std::make_shared<if_stmt>();
                        auto guard_statement_block = std::make_shared<stmt_block>();
                        guard_if_statement->then_stmt = guard_statement_block;
                        guard_if_statement->else_stmt = std::make_shared<stmt_block>();

                        // create guard if condition
                        auto guard_if_condition_expr = std::make_shared<var_expr>();
                        guard_if_condition_expr->var1 = guard_variables[i];

                        // set the guard if condition
                        guard_if_statement->cond = guard_if_condition_expr;

                        // insert set expressions inside if only if it is not the last guard block
                        if (guard_variables.size() >= 2 && i < guard_variables.size() - 1)
                            guard_statement_block->stmts.push_back(to<stmt>(set_guard_variable_exprs[i + 1].second));
                        
                        // insert break/continue inside the if statement based on the location of it
                        if (i < guard_variables.size() - 1)
                            guard_statement_block->stmts.push_back(std::make_shared<break_stmt>());
                        else
                            guard_statement_block->stmts.push_back(std::make_shared<continue_stmt>());
                        
                        guard_if_blocks.push_back(guard_if_statement);
                        guard_if_statement->dump(std::cerr, 0);
                    }

                    // first we need to set the innermost guard variable = 1;
                    ast->stmts.push_back(set_guard_variable_exprs[0].second);

                    // now we can take care of inserting other operations
                    // 1) insert guard variable definition
                    // 2) set guard variable = 0
                    // 3) inser the if guard block
                    unsigned i = 0;
                    if (use_mixed_tree) {
                        i = 0;
                    }
                    else {
                        i = 1;
                    }
                    for (unsigned blocks_counter = 0; i + 1 < loop_parent_tree.size() && blocks_counter < guard_variables.size(); i++, blocks_counter++) {
                        while_stmt::Ptr guard_set_insertion_loop = loop_parent_tree[i]->structured_ast_loop;
                        while_stmt::Ptr if_block_insertion_loop = loop_parent_tree[i + 1]->structured_ast_loop;

                        auto guard_set_insertion_block = to<stmt_block>(guard_set_insertion_loop->body);
                        auto if_block_insertion_block = to<stmt_block>(if_block_insertion_loop->body);

                        guard_set_insertion_block->stmts.insert(guard_set_insertion_block->stmts.begin(), to<stmt>(set_guard_variable_exprs[blocks_counter].first));

                        auto guard_insertion_loop = std::find(if_block_insertion_block->stmts.begin(), if_block_insertion_block->stmts.end(), to<stmt>(guard_set_insertion_loop));
                        if (guard_insertion_loop != if_block_insertion_block->stmts.end()) {
                            if_block_insertion_block->stmts.insert(guard_insertion_loop + 1, to<stmt>(guard_if_blocks[blocks_counter]));
                            if_block_insertion_block->stmts.insert(if_block_insertion_block->stmts.begin(), to<stmt>(guard_variable_defs[blocks_counter]));
                        }
                    }

                    std::cerr << "handling break cond\n";
                    is_goto_to_outerloop = true;
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

            std::cerr << "bb (open): " << bb->id << "\n";
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

                if (isa<goto_stmt>(bb->successor[0]->parent) && (int)bb->ast_depth - 2 > (int)bb->successor[0]->successor[0]->ast_depth) {
                    return_blocks.push_back({bb, ast});
                }
                else {
                    return_blocks.push_back({bb, nullptr});
                }
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
                    std::vector<std::pair<std::shared_ptr<basic_block>, stmt_block::Ptr>> loop_out_blocks;
                    ast->stmts.push_back(loop->convert_to_ast_impl(*this, dta, loop_out_blocks));
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

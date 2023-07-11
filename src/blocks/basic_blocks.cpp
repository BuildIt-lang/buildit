#include "blocks/basic_blocks.h"
#include <algorithm>

using namespace block;

basic_block::cfg_block generate_basic_blocks(block::stmt_block::Ptr ast) {
    std::deque<std::shared_ptr<basic_block>> work_list;
    basic_block::cfg_block return_list;
    int basic_block_count = 0;

    // step 1: fill the work_list
    for (auto st: ast->stmts) {
        auto bb = std::make_shared<basic_block>(std::to_string(basic_block_count));
        bb->parent = st;
        // bb->index = ;
        work_list.push_back(bb);
        basic_block_count++;
    }

    // step 2: add successors
    for (unsigned i = 0; work_list.size() != 0 && i < work_list.size() - 1; i++) {
        work_list[i]->successor.push_back(work_list[i+1]);
    }

    // step 3: process blocks: every xx_stmt type statement is made out into a basic block
    while (work_list.size()) {
        auto bb = work_list.front();

        if (isa<block::stmt_block>(bb->parent)) {
            stmt_block::Ptr stmt_block_ = to<stmt_block>(bb->parent);
            bb->name = "stmt" + bb->name;

            if (stmt_block_->stmts.size() > 0) {
                basic_block::cfg_block stmt_block_list;
                
                // convert all statements of this stmt_block into a basic block
                for (auto st: stmt_block_->stmts) {
                    stmt_block_list.push_back(std::make_shared<basic_block>(std::to_string(basic_block_count++)));
                    stmt_block_list.back()->parent = st;
                }
                
                // set the basic block successors
                for (unsigned i = 0; stmt_block_list.size() != 0 && i < stmt_block_list.size() - 1; i++) {
                    stmt_block_list[i]->successor.push_back(stmt_block_list[i+1]);
                }

                // since we insert these stmts between bb1 ---> bb2 ==> bb1 ---> (bb-a1...bb-an) ---> bb2
                // point the successor of the stmt_block_list to the basic block that bb1's successor 
                // pointed to. After this, clear the bb1's successor and push the front of stmt_block_list
                // to bb1's successor list.
                stmt_block_list.back()->successor.push_back(bb->successor.front());
                bb->successor.clear();
                bb->successor.push_back(stmt_block_list.front());
                
                // push a rather empty-ish basic block, which will branch to the next basic block, or the next statement.
                return_list.push_back(bb);
                work_list.pop_front();
                // now insert the pending blocks to be processed at the front of the work_list
                work_list.insert(work_list.begin(), stmt_block_list.begin(), stmt_block_list.end());
            }
            else {
                return_list.push_back(bb);
                work_list.pop_front();
            }
        }
        else if (isa<if_stmt>(bb->parent)) {
            bb->name = "if" + bb->name;

            if_stmt::Ptr if_stmt_ = to<if_stmt>(bb->parent);
            // assign the if condition to the basic block
            bb->branch_expr = if_stmt_->cond;
            
            // create a exit block
            auto exit_bb = std::make_shared<basic_block>("exit" + std::to_string(basic_block_count));
            // assign it a empty stmt_block as parent
            exit_bb->parent = std::make_shared<stmt_block>();
            // check if this is the last block, if yes the successor will be empty
            if (bb->successor.size()) {
                // set the successor to the block that if_stmt successor pointer to earlier
                exit_bb->successor.push_back(bb->successor.front());
                // clear the successor block from the if_stmt
                bb->successor.clear();
            }
            // remove the if from the work_list
            work_list.pop_front();
            // push the exit block to the work_list
            work_list.push_front(exit_bb);

            // if there is a then_stmt, create a basic block for it
            if (to<stmt_block>(if_stmt_->then_stmt)->stmts.size() != 0) {
                auto then_bb = std::make_shared<basic_block>(std::to_string(++basic_block_count));
                // set the parent of this block as the then stmts
                then_bb->parent = if_stmt_->then_stmt;
                // set the successor of this block to be the exit block
                then_bb->successor.push_back(exit_bb);
                // set the successor of the original if_stmt block to be this then block
                bb->successor.push_back(then_bb);
                // push the block to the work_list, to expand it further
                work_list.push_front(then_bb);
            }
            // if there is a else_stmt, create a basic block for it
            if (to<stmt_block>(if_stmt_->else_stmt)->stmts.size() != 0) {
                auto else_bb = std::make_shared<basic_block>(std::to_string(++basic_block_count));
                // set the parent of this block as the else stmts
                else_bb->parent = if_stmt_->else_stmt;
                // set the successor of this block to be the exit block
                else_bb->successor.push_back(exit_bb);
                // set the successor of the orignal if_stmt block to be this else block
                bb->successor.push_back(else_bb);
                // push the block to the work_list, to expand it further
                work_list.insert(work_list.begin() + 1, else_bb);
            }

            // if there is no else block, then have the exit block as successor as well.
            if (bb->successor.size() <= 1) bb->successor.push_back(exit_bb);

            return_list.push_back(bb);
        }
        else if (isa<block::expr_stmt>(bb->parent)) {
            bb->name = "expr" + bb->name;
            return_list.push_back(bb);
            work_list.pop_front();
        }
        else if (isa<block::decl_stmt>(bb->parent)) {
            bb->name = "decl" + bb->name;
            return_list.push_back(bb);
            work_list.pop_front();
        }
        else if (isa<block::label_stmt>(bb->parent)) {
            bb->name = "label" + bb->name;
            return_list.push_back(bb);
            work_list.pop_front();
        }
        else if (isa<block::goto_stmt>(bb->parent)) {
            bb->name = "goto" + bb->name;
            return_list.push_back(bb);
            work_list.pop_front();
        }
        else if (isa<block::return_stmt>(bb->parent)) {
            bb->name = "return" + bb->name;
            return_list.push_back(bb);
            work_list.pop_front();
        }

        basic_block_count++;
    }

    // step 4: resolve goto calls to successors of labels
    for (auto bb: return_list) {
        if (isa<block::goto_stmt>(bb->parent)) {
            auto goto_source = std::find_if(return_list.begin(), return_list.end(), 
                [bb](std::shared_ptr<basic_block> bb_l) {
                    if (isa<label_stmt>(bb_l->parent)) {
                        return to<label_stmt>(bb_l->parent)->label1 == to<goto_stmt>(bb->parent)->label1;
                    }
                    return false;
                });
            if (goto_source != return_list.end()) {
                bb->successor.clear();
                bb->successor.push_back(*goto_source);
            }
        }
    }

    // step 5: populate the predecessors
    for (auto bb: return_list) {
        for (auto succ: bb->successor) {
            succ->predecessor.push_back(bb);
        }
    }

    // step 6: assign each basic_block an id
    for (unsigned int i = 0; i < return_list.size(); i++) {
        return_list[i]->id = i;
    }

    return return_list;
}
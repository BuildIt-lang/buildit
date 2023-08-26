#ifndef LOOP_H
#define LOOP_H
#include "blocks/block_visitor.h"
#include "blocks/basic_blocks.h"
#include "blocks/dominance.h"
#include "blocks/stmt.h"
#include <unordered_set>

using namespace block;
class loop {
public:
    loop(std::shared_ptr<basic_block> header): header_block(header) {}

// private:
    struct loop_bounds_ {
        stmt::Ptr ind_var;
        // MISS: intial value of ind var
        stmt::Ptr steps_ind_var;
        // MISS: value of the step
        // MISS: final value of the step
        stmt::Ptr cond_ind_var;
        // MISS: direction of step
        stmt::Ptr entry_stmt;
    } loop_bounds;

    unsigned int loop_id;
    basic_block::cfg_block blocks;
    std::unordered_set<int> blocks_id_map;
    std::shared_ptr<loop> parent_loop;
    std::shared_ptr<basic_block> header_block;
    basic_block::cfg_block loop_latch_blocks;
    std::vector<std::shared_ptr<loop>> subloops;
};

class loop_info {
public:
    loop_info(basic_block::cfg_block ast, dominator_analysis &dt): parent_ast(ast), dta(dt) {
        analyze();
    }
    std::shared_ptr<loop> allocate_loop(std::shared_ptr<basic_block> header);
    block::stmt_block::Ptr convert_to_ast(block::stmt_block::Ptr ast);
    std::map<unsigned int, std::vector<int>> postorder_loops_map;
    std::vector<std::shared_ptr<loop>> loops;
    std::vector<std::shared_ptr<loop>> top_level_loops;

private:
    basic_block::cfg_block parent_ast;
    dominator_analysis dta;
    std::map<int, std::shared_ptr<loop>> bb_loop_map;
    void postorder_dfs_helper(std::vector<int> &postorder_loops_map, std::vector<bool> &visited_loops, int id);
    // discover loops during traversal of the abstract syntax tree
    void analyze();
};

#endif
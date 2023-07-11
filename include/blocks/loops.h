#ifndef LOOP_H
#define LOOP_H
#include "blocks/block_visitor.h"
#include "blocks/basic_blocks.h"
#include "blocks/dominance.h"
#include "blocks/stmt.h"

using namespace block;
class loop {
public:
    loop() = default;

private:
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

    basic_block::cfg_block exit_bbs;
};

class loop_info {
public:
    loop_info(basic_block::cfg_block ast, dominator_analysis dt): parent_ast(ast), dta(dt) {
        analyze();
    }
private:
    basic_block::cfg_block parent_ast;
    dominator_analysis dta;
    // discover loops during traversal of the abstract syntax tree
    void analyze();
};

#endif
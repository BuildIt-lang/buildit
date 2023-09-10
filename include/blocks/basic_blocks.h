#ifndef BASIC_BLOCKS_H
#define BASIC_BLOCKS_H
#include "blocks/stmt.h"
#include <vector>
#include <deque>
#include <string>
#include <map>

class basic_block {
    public:
        typedef std::vector<std::shared_ptr<basic_block>> cfg_block;
        basic_block(std::string label): name(label) {};
        // only does a shallow copy (leaves out predecessors and successors)
        // basic_block(basic_block &bb) {
        //     bb.branch_expr = branch_expr;
        //     bb.then_branch = then_branch;
        //     bb.else_branch = else_branch;
        //     bb.parent = parent;
        //     bb.ast_index = ast_index;
        //     bb.ast_depth = ast_depth;
        //     bb.id = id;
        //     bb.name = name;
        // }

        cfg_block predecessor;
        cfg_block successor;
        block::expr::Ptr branch_expr;
        std::shared_ptr<basic_block> then_branch;
        std::shared_ptr<basic_block> else_branch;
        block::stmt::Ptr parent;
        unsigned int ast_index;
        unsigned int ast_depth;
        unsigned int id;
        std::string name;
        // static std::map<block::stmt::Ptr, std::shared_ptr<basic_block>> ast_to_basic_block_map;
};

basic_block::cfg_block generate_basic_blocks(block::stmt_block::Ptr ast);

#endif
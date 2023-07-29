#ifndef BASIC_BLOCKS_H
#define BASIC_BLOCKS_H
#include "blocks/stmt.h"
#include <vector>
#include <deque>
#include <string>

class basic_block {
    public:
        typedef std::vector<std::shared_ptr<basic_block>> cfg_block;
        basic_block(std::string label): name(label) {};

        cfg_block predecessor;
        cfg_block successor;
        block::expr::Ptr branch_expr;
        block::stmt::Ptr parent;
        unsigned int ast_index;
        unsigned int id;
        std::string name;
};

basic_block::cfg_block generate_basic_blocks(block::stmt_block::Ptr ast);

#endif
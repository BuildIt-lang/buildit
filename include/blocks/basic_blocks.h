#ifndef BASIC_BLOCKS_H
#define BASIC_BLOCKS_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"
#include <vector>
#include <deque>
#include <string>

class basic_block {
    public:
        basic_block(std::string label): name(label) {};

        std::vector<std::shared_ptr<basic_block>> predecessor;
        std::vector<std::shared_ptr<basic_block>> successor;
        block::expr::Ptr branch_expr;
        block::stmt::Ptr parent;
        unsigned int index;
        std::string name;
};

std::vector<std::shared_ptr<basic_block>> generate_basic_blocks(block::stmt_block::Ptr ast);

#endif
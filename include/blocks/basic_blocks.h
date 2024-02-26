#ifndef BASIC_BLOCKS_H
#define BASIC_BLOCKS_H
#include "blocks/stmt.h"
#include <vector>
#include <deque>
#include <string>
#include <map>

namespace block {
class basic_block {
    public:
        typedef std::vector<std::shared_ptr<basic_block>> cfg_block;
        basic_block(std::string label): name(label) {};

        cfg_block predecessor;
        cfg_block successor;
        expr::Ptr branch_expr;
        std::shared_ptr<basic_block> then_branch;
        std::shared_ptr<basic_block> else_branch;
        std::shared_ptr<basic_block> exit_block;
        bool is_exit_block = false;
        stmt::Ptr parent;
        unsigned int ast_index;
        unsigned int ast_depth;
        unsigned int id;
        std::string name;
        static std::map<stmt::Ptr, std::shared_ptr<basic_block>> ast_to_basic_block_map;
};

basic_block::cfg_block generate_basic_blocks(stmt_block::Ptr ast);
void dump(basic_block::cfg_block basic_block_list);
} // namespace block

#endif
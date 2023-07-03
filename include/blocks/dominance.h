#ifndef DOMINANCE_H
#define DOMINANCE_H
#include "blocks/block_visitor.h"
#include "blocks/basic_blocks.h"
#include "blocks/stmt.h"
#include <vector>
#include <stack>
#include <string>
#include <bitset>
#include <algorithm>

class dominator_tree {
    public:
        dominator_tree(std::vector<std::shared_ptr<basic_block>> &cfg);
        std::vector<int> &get_postorder_bb_map();
        std::vector<int> &get_postorder();
        std::vector<int> &get_idom();
        int get_idom(int bb_id);
        bool dominates(int bb1_id, int bb2_id);
        bool is_reachable_from_entry(int bb_id);
        void analyze();

    private:
        std::vector<int> idom;
        std::vector<int> postorder;
        std::vector<int> postorder_bb_map;
        std::vector<std::shared_ptr<basic_block>> &cfg_;
        void postorder_dfs_helper(std::vector<bool> &visited_bbs, int id);
        void postorder_dfs();
        int intersect(int bb1_id, int bb2_id);
};


#endif
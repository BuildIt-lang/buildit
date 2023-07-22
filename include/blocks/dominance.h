#ifndef DOMINANCE_H
#define DOMINANCE_H
#include "blocks/block_visitor.h"
#include "blocks/basic_blocks.h"
#include "blocks/stmt.h"
#include <map>
#include <vector>
#include <stack>
#include <string>
#include <bitset>
#include <algorithm>

class dominator_analysis {
    public:
        // struct dominator_tree_ {
        //     dominator_tree_(int id): bb_id(id);
        //     int bb_id;
        //     std::vector<std::shared_ptr<dominator_tree_>> child_nodes;
        // } dominator_tree;
        dominator_analysis(basic_block::cfg_block &cfg);
        basic_block::cfg_block &cfg_;
        std::vector<int> &get_postorder_bb_map();
        std::vector<int> &get_postorder();
        std::vector<int> &get_idom();
        std::map<int, std::vector<int>> &get_idom_map();
        std::vector<int> &get_postorder_idom_map();
        int get_idom(int bb_id);
        std::vector<int> get_idom_map(int bb_id);
        int get_postorder_idom_map(int idom_id);
        bool dominates(int bb1_id, int bb2_id);
        bool is_reachable_from_entry(int bb_id);
        void analyze();

    private:
        std::vector<int> idom;
        std::map<int, std::vector<int>> idom_map;
        std::vector<int> postorder_idom;
        std::vector<int> postorder;
        std::vector<int> postorder_bb_map;
        void postorder_idom_helper(std::vector<bool> &visited, int id);
        void postorder_dfs_helper(std::vector<bool> &visited_bbs, int id);
        void postorder_dfs();
        int intersect(int bb1_id, int bb2_id);
};


#endif
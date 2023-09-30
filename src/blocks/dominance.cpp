#include "blocks/dominance.h"

using namespace block;

void dominator_analysis::reverse_cfg() {
    // TODO: Add a check for size, it should be greater than 2.
    if (cfg_.size() == 0)
        assert(0);

    std::shared_ptr<basic_block> virtual_exit_bb = std::make_shared<basic_block>("virtualexit0");
    for (auto bb: cfg_) {
        if (bb->successor.size() == 0) {
            bb->successor.push_back(virtual_exit_bb);
            virtual_exit_bb->predecessor.push_back(bb);
        }
    }

    // if CFG is an inifite loop, we don't have a exit block
    // so we need to find the farthest block from the entry
    // of the loop and consider that as one of the exit blocks
    if (!virtual_exit_bb->predecessor.size()) {
        std::cerr << "infinite loop\n";
        postorder_dfs(false);

        auto bb_virtual_backedge = cfg_[max_depth_bb_id];
        bb_virtual_backedge->successor.push_back(virtual_exit_bb);
        virtual_exit_bb->predecessor.push_back(bb_virtual_backedge);
    }

    virtual_exit_bb->id = cfg_.size();
    cfg_.push_back(virtual_exit_bb);

    for (auto bb: cfg_) {
        basic_block::cfg_block temp_pred = bb->predecessor;
        bb->predecessor.clear();
        bb->predecessor.insert(bb->predecessor.begin(), bb->successor.begin(), bb->successor.end());
        std::reverse(bb->predecessor.begin(), bb->predecessor.end());
        bb->successor.clear();
        bb->successor.insert(bb->successor.begin(), temp_pred.begin(), temp_pred.end());
        std::reverse(bb->successor.begin(), bb->successor.end());
    }
}

dominator_analysis::dominator_analysis(basic_block::cfg_block cfg, bool is_postdom) : cfg_(cfg), is_postdom_(is_postdom) {
    if (is_postdom) {
        reverse_cfg();
    }

    // TODO: Add a check for size, it should be greater than 2.
    idom.clear();
    idom.reserve(cfg_.size());
    idom.assign(cfg_.size(), -1);
    postorder.clear();
    postorder.reserve(cfg_.size());
    postorder_bb_map.clear();
    postorder_bb_map.reserve(cfg_.size());
    postorder_bb_map.assign(cfg_.size(), -1);
    preorder.clear();
    preorder.reserve(cfg_.size());
    preorder_bb_map.clear();
    preorder_bb_map.reserve(cfg_.size());
    preorder_bb_map.assign(cfg_.size(), -1);

    // and call the anaylse function
    analyze();
}

void dominator_analysis::postorder_idom_helper(std::vector<bool> &visited, int id) {
    for (int idom_id: idom_map[id]) {
        if (idom_id != -1 && !visited[idom_id]) {
            visited[idom_id] = true;
            postorder_idom_helper(visited, idom_id);
            postorder_idom.push_back(idom_id);
        }
    }
}

void dominator_analysis::postorder_dfs_helper(std::vector<bool> &visited_bbs, int id, int depth) {
    if (depth > max_depth) {
        max_depth = depth;
        max_depth_bb_id = id;
    }

    for (auto child: cfg_[id]->successor) {
        if (!visited_bbs[child->id]) {
            visited_bbs[child->id] = true;
            postorder_dfs_helper(visited_bbs, child->id, depth + 1);
            postorder.push_back(child->id);
        }
    }
}

void dominator_analysis::postorder_dfs(bool reverse_cfg) {
    int current_depth = 0;
    max_depth = current_depth;

    std::vector<bool> visited_bbs(cfg_.size());
    visited_bbs.assign(visited_bbs.size(), false);
    if (reverse_cfg)
        visited_bbs[cfg_.size() - 1] = true;
    else
        visited_bbs[0] = true;

    if (reverse_cfg) {
        max_depth_bb_id = cfg_.size() - 1;
        postorder_dfs_helper(visited_bbs, cfg_.size() - 1, current_depth + 1);
        postorder.push_back(cfg_.size() - 1);
    }
    else {
        max_depth_bb_id = 0;
        postorder_dfs_helper(visited_bbs, 0, current_depth + 1);
        postorder.push_back(0);
    }
}

void dominator_analysis::preorder_dfs_helper(std::vector<bool> &visited_bbs, int id) {
        for (auto child: cfg_[id]->successor) {
            if (!visited_bbs[child->id]) {
                visited_bbs[child->id] = true;
                preorder.push_back(child->id);
                preorder_dfs_helper(visited_bbs, child->id);
            }
        }
}

void dominator_analysis::preorder_dfs(bool reverse_cfg) {
    std::vector<bool> visited_bbs(cfg_.size());
    visited_bbs.assign(visited_bbs.size(), false);
    if (reverse_cfg)
        visited_bbs[cfg_.size() - 1] = true;
    else
        visited_bbs[0] = true;

    if (reverse_cfg) {
        preorder.push_back(cfg_.size() - 1);
        preorder_dfs_helper(visited_bbs, cfg_.size() - 1);
    }
    else {
        preorder.push_back(0);
        preorder_dfs_helper(visited_bbs, 0);
    }
}

std::vector<int> &dominator_analysis::get_postorder_bb_map() {
    return postorder_bb_map;
}

std::vector<int> &dominator_analysis::get_postorder() {
    return postorder;
}

std::vector<int> &dominator_analysis::get_preorder_bb_map() {
    return preorder_bb_map;
}

std::vector<int> &dominator_analysis::get_preorder() {
    return preorder;
}

std::vector<int> &dominator_analysis::get_idom() {
    return idom;
}

std::map<int, std::vector<int>> &dominator_analysis::get_idom_map() {
    return idom_map;
}

std::vector<int> &dominator_analysis::get_postorder_idom_map() {
    return postorder_idom;
}

int dominator_analysis::get_idom(int bb_id) {
    if (bb_id < 0 || bb_id >= (int)idom.size()) {
        return -1;
    }

    return idom[bb_id];
}

std::vector<int> dominator_analysis::get_idom_map(int bb_id) {
    if (bb_id < 0 || bb_id >= (int)idom_map.size()) {
        return {};
    }

    return idom_map[bb_id];
}

int dominator_analysis::get_postorder_idom_map(int idom_id) {
    if (idom_id < 0 || idom_id >= (int)postorder_idom.size()) {
        return -1;
    }

    return postorder_idom[idom_id];
}

bool dominator_analysis::dominates(int bb1_id, int bb2_id) {
    if (bb1_id == 0) {
        return true;
    }

    int pointer = idom[bb2_id];
    while (pointer != 0) {
        if (pointer == bb1_id) {
            return true;
        }
        pointer = idom[pointer];
    }

    return false;
}

bool dominator_analysis::is_reachable_from_entry(int bb_id) {
    return dominates(0, bb_id);
}

int dominator_analysis::intersect(int bb1_id, int bb2_id) {
    while (bb1_id != bb2_id) {
        if (postorder_bb_map[bb1_id] < postorder_bb_map[bb2_id]) {
            bb1_id = idom[bb1_id];
        }
        else {
            bb2_id = idom[bb2_id];
        }
    }

    return bb1_id;
}

void dominator_analysis::analyze() {
    preorder_dfs(is_postdom_);
    postorder_dfs(is_postdom_); 
    for (unsigned int i = 0; i < preorder.size(); i++) {
        preorder_bb_map[preorder[i]] = i;
    }
    for (unsigned int i = 0; i < postorder.size(); i++) {
        postorder_bb_map[postorder[i]] = i;
    }
    if (is_postdom_)
        idom[cfg_.size() - 1] = cfg_.size() - 1;
    else
        idom[0] = 0;

    bool change = false;

    do {
        change = 0;
        for (int i = postorder.size() - 2; i >= 0; i--) {
            int postorder_bb_num = postorder[i];
            std::shared_ptr<basic_block> bb = cfg_[postorder_bb_num];
            int bb_id_idom = bb->predecessor[0]->id;

            for (unsigned int j = 1; j < bb->predecessor.size(); j++) {
                int bb_id_idom_next = bb->predecessor[j]->id;
                if (idom[bb_id_idom_next] != -1) {
                    bb_id_idom = intersect(bb_id_idom, bb_id_idom_next);
                }
            }

            if (idom[postorder_bb_num] != bb_id_idom) {
                idom[postorder_bb_num] = bb_id_idom;
                change = 1;
            }
        }
    } while(change);

    // build a map of dominators for easy traversal.
    for (unsigned int i = 0; i < idom.size(); i++) {
        idom_map[idom[i]].push_back(i);
    }

    for (unsigned int i = 0; i < idom.size(); i++) {
        if (idom_map[i].empty())
            idom_map[i].push_back(-1);
    }

    // build a postorder visit list of idom_tree
    std::vector<bool> visited_idom_nodes(idom_map.size());
    visited_idom_nodes.assign(visited_idom_nodes.size(), false);
    visited_idom_nodes[0] = true;

    postorder_idom_helper(visited_idom_nodes, 0);
    postorder_idom.push_back(0);
}
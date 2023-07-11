#include "blocks/dominance.h"

using namespace block;

dominator_analysis::dominator_analysis(basic_block::cfg_block &cfg) : cfg_(cfg) {
    // TODO: Add a check for size, it should be greater than 2.
    idom.reserve(cfg_.size());
    idom.assign(cfg_.size(), -1);
    postorder.reserve(cfg_.size());
    postorder_bb_map.reserve(cfg_.size());
    postorder_bb_map.assign(cfg_.size(), -1);

    // and call the anaylse function
    analyze();
}

void dominator_analysis::postorder_dfs_helper(std::vector<bool> &visited_bbs, int id) {
    for (auto child: cfg_[id]->successor) {
        if (!visited_bbs[child->id]) {
            visited_bbs[child->id] = true;
            postorder_dfs_helper(visited_bbs, child->id);
            postorder.push_back(child->id);
        }
    }    
}
void dominator_analysis::postorder_dfs() {
    std::vector<bool> visited_bbs(cfg_.size());
    visited_bbs.assign(visited_bbs.size(), false);
    visited_bbs[0] = true;

    postorder_dfs_helper(visited_bbs, 0);
    postorder.push_back(0);
}

std::vector<int> &dominator_analysis::get_postorder_bb_map() {
    return postorder_bb_map;
}

std::vector<int> &dominator_analysis::get_postorder() {
    return postorder;
}

std::vector<int> &dominator_analysis::get_idom() {
    return idom;
}

std::map<int, std::vector<int>> &dominator_analysis::get_idom_map() {
    return idom_map;
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
    postorder_dfs();
    for (unsigned int i = 0; i < postorder.size(); i++) {
        postorder_bb_map[postorder[i]] = i;
    }

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

    // for (auto key: idom_map) {
    //     std::cout << key.first << ": ";
    //     for (int id: key.second) {
    //         std::cout << id << " ";
    //     }
    //     std::cout << "\n";
    // }
}
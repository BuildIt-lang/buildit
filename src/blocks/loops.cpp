#include "blocks/loops.h"
#include <algorithm>

void loop_info::analyze() {
    std::vector<int> idom = dta.get_idom();
    for (unsigned int i = 0; i < idom.size(); i++) {
        std::cout << i << " : " << idom[i] << "\n";
    }
}

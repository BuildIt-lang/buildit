#ifndef BLOCKS_MATCHERS_H
#define BLOCKS_MATCHERS_H
#include "blocks/matchers/patterns.h"
#include "blocks/block.h"
#include <map>

namespace block {
namespace matcher {

struct match {
	block::Ptr node;
	std::map<std::string, block::Ptr> captures;
};

std::vector<match> find_all_matches(std::shared_ptr<pattern> p, block::Ptr node);
bool check_match(std::shared_ptr<pattern> p, block::Ptr node, std::map<std::string, block::Ptr>& captures);

}
}


#endif

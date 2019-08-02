#ifndef PARENT_FINDER_H
#define PARENT_FINDER_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"

namespace block {
class parent_finder: public block_visitor {
public:
	stmt::Ptr to_find;
	stmt_block::Ptr found_parent = nullptr;
	virtual void visit(stmt_block::Ptr);
	
};

}
#endif

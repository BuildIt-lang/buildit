#ifndef BLOCKS_GENERIC_CHECKER_H
#define BLOCKS_GENERIC_CHECKER_H

#include "blocks/block_visitor.h"
#include "blocks/var.h"

namespace block {

class generic_null_checker: public block_visitor {
	using block_visitor::visit;

	void visit(var::Ptr) override;
};

}


#endif

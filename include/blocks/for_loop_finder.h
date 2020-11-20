#ifndef FOR_LOOP_FINDER_H
#define FOR_LOOP_FINDER_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"

namespace block {
class for_loop_finder : public block_visitor {
public:
	using block_visitor::visit;
	stmt::Ptr ast;
	virtual void visit(stmt_block::Ptr) override;
};
} // namespace block
#endif

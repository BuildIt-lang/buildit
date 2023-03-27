#ifndef SUB_EXPR_CLEANUP_H
#define SUB_EXPR_CLEANUP_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"
namespace block {
class sub_expr_cleanup : public block_visitor {
public:
	using block_visitor::visit;
	virtual void visit(stmt_block::Ptr);
};
} // namespace block
#endif

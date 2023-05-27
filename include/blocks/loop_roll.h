#ifndef BLOCK_LOOP_ROLL_H
#define BLOCK_LOOP_ROLL_H
#include "blocks/block_replacer.h"
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"

namespace block {
class loop_roll_finder : public block_visitor {
public:
	using block_visitor::visit;
	virtual void visit(stmt_block::Ptr);
};

class constant_expr_finder : public block_visitor {
public:
	using block_visitor::visit;
	std::vector<const_expr::Ptr> constants;

	// Handle only int constants for now
	virtual void visit(int_const::Ptr);
};

class constant_replacer : public block_replacer {
public:
	using block_visitor::visit;
	std::vector<expr::Ptr> replace;
	int curr_index = 0;

	virtual void visit(int_const::Ptr);

	/*
	// Handle plus expr for now
	// Add more expressions later or use expression rewriter instead
	virtual void visit(plus_expr::Ptr);
	virtual void visit(mul_expr::Ptr);
	virtual void visit(minus_expr::Ptr);
	virtual void visit(div_expr::Ptr);
	virtual void visit(sq_bkt_expr::Ptr);
	*/
};

} // namespace block

#endif

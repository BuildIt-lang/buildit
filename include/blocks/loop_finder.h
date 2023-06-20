#ifndef LOOP_FINDER_H
#define LOOP_FINDER_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"

namespace block {
class loop_finder : public block_visitor {
public:
	using block_visitor::visit;
	stmt::Ptr ast;

	int loop_hook_counter = 0;
	void visit_label(label_stmt::Ptr, stmt_block::Ptr);
	virtual void visit(stmt_block::Ptr);
};
class last_jump_finder : public block_visitor {
public:
	using block_visitor::visit;
	bool has_jump_to = false;
	label::Ptr jump_label;

	virtual void visit(goto_stmt::Ptr);
};

class continue_finder : public block_visitor {
public:
	using block_visitor::visit;
	bool has_continue = false;
	virtual void visit(continue_stmt::Ptr);
};

class outer_jump_finder : public block_visitor {
public:
	using block_visitor::visit;
	int &loop_hook_counter;
	outer_jump_finder(int &lc) : loop_hook_counter(lc) {}
	std::vector<std::pair<var::Ptr, stmt::Ptr>> created_vars;
	virtual void visit(stmt_block::Ptr);
};
} // namespace block
#endif

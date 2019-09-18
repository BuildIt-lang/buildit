#ifndef LOOP_FINDER_H
#define LOOP_FINDER_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"

namespace block {
class loop_finder: public block_visitor {
public:
	using block_visitor::visit;
	stmt::Ptr ast;
	void visit_label(label_stmt::Ptr, stmt_block::Ptr);
	virtual void visit(stmt_block::Ptr);
	
};
class last_jump_finder: public block_visitor {
public:
	using block_visitor::visit;
	bool has_jump_to = false;
	label::Ptr jump_label;
	
	virtual void visit(goto_stmt::Ptr);	
};

}
#endif

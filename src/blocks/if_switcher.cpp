#include "blocks/if_switcher.h"

namespace block {


class continue_counter: public block_visitor {
public:
	int count = 0;
	using block_visitor::visit;
	virtual void visit(continue_stmt::Ptr) {
		count++;
	}

	// Do not recurse for loops
	virtual void visit(while_stmt::Ptr) {
		return;
	}
	virtual void visit(for_stmt::Ptr) {
		return;
	}
};


static stmt::Ptr fix_loop_inversion(if_stmt::Ptr ifs) {

	if (!isa<stmt_block>(ifs->then_stmt))
		return ifs;
	
	auto then_block = to<stmt_block>(ifs->then_stmt);
	auto else_block = to<stmt_block>(ifs->else_stmt);
	
	if (then_block->stmts.size() != 1 || else_block->stmts.size() != 0) 
		return ifs;

	if (!isa<while_stmt>(then_block->stmts[0]))
		return ifs;
	
	auto ws = to<while_stmt>(then_block->stmts[0]);

	if (!isa<int_const>(ws->cond))
		return ifs;

	if (!(to<int_const>(ws->cond)->value == 1))
		return ifs;

	if (!isa<stmt_block>(ws->body))
		return ifs;
	
	if (to<stmt_block>(ws->body)->stmts.size() == 0) 
		return ifs;
	
	auto last_stmt = to<stmt_block>(ws->body)->stmts.back();
	if (!isa<if_stmt>(last_stmt))
		return ifs;

	if (!isa<not_expr>(to<if_stmt>(last_stmt)->cond))
		return ifs;
	if (!to<not_expr>(to<if_stmt>(last_stmt)->cond)->expr1->is_same(ifs->cond))
		return ifs;

	auto nfs = to<if_stmt>(last_stmt);

	if (!isa<stmt_block>(nfs->then_stmt) || to<stmt_block>(nfs->then_stmt)->stmts.size() != 1)
		return ifs;
	if (!isa<stmt_block>(nfs->else_stmt) || to<stmt_block>(nfs->else_stmt)->stmts.size() != 0)
		return ifs;

	if (!isa<break_stmt>(to<stmt_block>(nfs->then_stmt)->stmts[0]))
		return ifs;	
	
	// make sure ws has no continues
	continue_counter counter;
	ws->accept(&counter);
	if (counter.count != 0)
		return ifs;


	// everything looks good, time to patch
	ws->cond = ifs->cond;
	to<stmt_block>(ws->body)->stmts.pop_back();
	
	return ws;
}



void if_switcher::visit(if_stmt::Ptr ifs) {
	if (isa<stmt_block>(ifs->then_stmt)) {
		auto then_block = to<stmt_block>(ifs->then_stmt);
		auto else_block = to<stmt_block>(ifs->else_stmt);
		if (then_block->stmts.size() == 0 && else_block->stmts.size() != 0) {
			ifs->then_stmt = ifs->else_stmt;
			ifs->else_stmt = then_block;
			auto ne = std::make_shared<not_expr>();
			ne->expr1 = ifs->cond;
			ifs->cond = ne;
		}
		
	}
	block_replacer::visit(ifs);
	
	node = fix_loop_inversion(ifs);
}




} // namespace block

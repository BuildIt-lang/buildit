#include "blocks/if_switcher.h"

namespace block {

void if_switcher::visit(if_stmt::Ptr ifs) {
	if (isa<stmt_block>(ifs->then_stmt)) {
		auto then_block = to<stmt_block>(ifs->then_stmt);
		if (then_block->stmts.size() == 0) {
			ifs->then_stmt = ifs->else_stmt;
			ifs->else_stmt = then_block;
			auto ne = std::make_shared<not_expr>();
			ne->expr1 = ifs->cond;
			ifs->cond = ne;
		}
	}
	block_visitor::visit(ifs);
}

} // namespace block

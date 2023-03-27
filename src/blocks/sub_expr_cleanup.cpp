#include "blocks/sub_expr_cleanup.h"

namespace block {

void sub_expr_cleanup::visit(stmt_block::Ptr sb) {
	std::vector<stmt::Ptr> new_stmts;
	for (auto stmt : sb->stmts) {
		if (!isa<expr_stmt>(stmt) || !to<expr_stmt>(stmt)->mark_for_deletion) {
			new_stmts.push_back(stmt);
		}
	}
	sb->stmts = new_stmts;
	block_visitor::visit(sb);
}

} // namespace block

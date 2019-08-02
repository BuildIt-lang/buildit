#include "blocks/parent_finder.h"

namespace block {
void parent_finder::visit(stmt_block::Ptr a) {
	if (found_parent != nullptr)
		return;
	for (auto stmt: a->stmts) {
		if (stmt == to_find) {
			found_parent = a;
			return;
		}	
		stmt->accept(this);
	}
}
}

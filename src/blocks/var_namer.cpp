#include "blocks/var_namer.h"

namespace block {
void var_namer::visit(decl_stmt::Ptr a) {
	a->decl_var->var_name = "var" + std::to_string(var_counter);
	var_counter++;
	var_replacer replacer;
	replacer.to_replace = a->decl_var;
	replacer.offset_to_replace = a->decl_var->static_offset;
	ast->accept(&replacer);
}
/*
void var_replacer::visit(assign_expr::Ptr a) {
	if (a->var1->static_offset == offset_to_replace) {
		a->var1 = to_replace;
	}
	a->expr1->accept(this);
}
*/
void var_replacer::visit(var_expr::Ptr a) {
	if (a->var1->static_offset == offset_to_replace) {
		a->var1 = to_replace;
	}
}
} // namespace block

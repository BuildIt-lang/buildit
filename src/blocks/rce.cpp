#include "blocks/rce.h"

namespace block {

void gather_redundant_decls::visit(decl_stmt::Ptr decl) {
	if (decl->init_expr == nullptr)
		return;
	if (isa<var_expr>(decl->init_expr))
		gathered_decls.push_back(decl);	
}

void gather_redundant_decls::visit(assign_expr::Ptr assign) {
	if (!isa<var_expr>(assign->var1))
		return;

	var::Ptr used_var = to<var_expr>(assign->var1)->var1;

	std::vector<decl_stmt::Ptr> new_gathers;
	for (auto decl: gathered_decls) {
		if (decl->decl_var == used_var)
			continue;
		new_gathers.push_back(decl);
	}
	
	gathered_decls = new_gathers;
}

void replace_redundant_vars::visit(var_expr::Ptr e) {
	if (e->var1 == to_replace->decl_var) {
		var::Ptr new_var = to<var_expr>(to_replace->init_expr)->var1;
		e->var1 = new_var;
	}
}

void replace_redundant_vars::visit(stmt_block::Ptr block) {
	std::vector<stmt::Ptr> new_stmts;
	for (auto stmt: block->stmts) {
		if (stmt == to_replace)
			continue;
		stmt->accept(this);
		new_stmts.push_back(stmt);
	}
	block->stmts = new_stmts;
}

void eliminate_redundant_vars(block::Ptr ast) {
	gather_redundant_decls gather;
	ast->accept(&gather);
	while (gather.gathered_decls.size()) {
		decl_stmt::Ptr to_replace = gather.gathered_decls.back();
		gather.gathered_decls.pop_back();
		
		replace_redundant_vars replacer;
		replacer.to_replace = to_replace;
		
		ast->accept(&replacer);
	}
}


}

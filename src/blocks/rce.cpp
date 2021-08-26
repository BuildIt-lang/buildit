#include "blocks/rce.h"

namespace block {

void check_side_effects::visit(assign_expr::Ptr) {
	has_side_effects = true;
}
void check_side_effects::visit(function_call_expr::Ptr) {
	has_side_effects = true;
}
void use_counter::visit(var_expr::Ptr a) {
	if (a->var1 == to_find)
		total_uses++;
}
void gather_redundant_decls::visit(decl_stmt::Ptr decl) {	
	if (decl->init_expr == nullptr)
		return;
	// Let us only replace those variables that are either simple variables on the RHS
	// Or are used exactly once
	if (isa<var_expr>(decl->init_expr)) {
		gathered_decls.push_back(decl);
		return;
	}
	use_counter counter;
	counter.to_find = decl->decl_var;
	ast->accept(&counter);
	if (counter.total_uses == 1) {	
		check_side_effects checker;
		decl->init_expr->accept(&checker);
		if (checker.has_side_effects == false)
			gathered_decls.push_back(decl);
	}
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
		node = to_replace->init_expr;
	} else {
		node = e;
	}
}

void replace_redundant_vars::visit(stmt_block::Ptr block) {
	std::vector<stmt::Ptr> new_stmts;
	for (auto s: block->stmts) {
		if (s == to_replace)
			continue;
		auto tmp = rewrite<stmt>(s);
		new_stmts.push_back(tmp);
	}
	block->stmts = new_stmts;	
	node = block;
}

void eliminate_redundant_vars(block::Ptr ast) {
	gather_redundant_decls gather;
	gather.ast = ast;
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

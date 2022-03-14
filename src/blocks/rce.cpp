#include "blocks/rce.h"

namespace block {

void check_side_effects::visit(assign_expr::Ptr) {
	has_side_effects = true;
}
void check_side_effects::visit(function_call_expr::Ptr) {
	has_side_effects = true;
}
void check_side_effects::visit(addr_of_expr::Ptr) {
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
		//current_decl = decl;
		gathered_decls.push_back(decl);
		return;
	}
	use_counter counter;
	counter.to_find = decl->decl_var;
	ast->accept(&counter);
	if (counter.total_uses <= 1) {	
		
		check_side_effects checker;
		decl->init_expr->accept(&checker);
		if (counter.total_uses == 0 || checker.has_side_effects == false) {
			gathered_decls.push_back(decl);
		}
	}
}

void gather_redundant_decls::visit(assign_expr::Ptr assign) {
	std::vector<decl_stmt::Ptr> new_gathers;
	
	if (!isa<var_expr>(assign->var1))
		return;
	var_expr::Ptr var1 = to<var_expr>(assign->var1);	

	for (auto a: gathered_decls) {
		if (a->decl_var == var1->var1)
			continue;
		new_gathers.push_back(a);
	}
	
	gathered_decls = new_gathers;
}

void replace_redundant_vars::visit(var_expr::Ptr e) {
	if (!decl_found) {
		node = e;
		return;
	}
	if (e->var1 == to_replace->decl_var) {
		node = to_replace->init_expr;
		has_replaced = true;
	} else {
		node = e;
	}
}
void replace_redundant_vars::visit(decl_stmt::Ptr decl) {
	if (decl->init_expr) 
		decl->init_expr = rewrite(decl->init_expr);
	if (decl->decl_var == to_replace->decl_var) 
		decl_found = true;
	node = decl;
}

void replace_redundant_vars::visit(assign_expr::Ptr assign) {
	assign->expr1 = rewrite(assign->expr1);
	assign->var1 = rewrite(assign->var1);

	if (!isa<var_expr>(to_replace->init_expr))
		decl_found = false;
	node = assign;
}
void replace_redundant_vars::visit(addr_of_expr::Ptr addr) {
	addr->expr1 = rewrite(addr->expr1);
	if (!isa<var_expr>(to_replace->init_expr))
		decl_found = false;
	node = addr;
}
void replace_redundant_vars::visit(function_call_expr::Ptr f) {
	for (unsigned int i = 0; i < f->args.size(); i++) {
		f->args[i] = rewrite(f->args[i]);
	}
	if (!isa<var_expr>(to_replace->init_expr))
		decl_found = false;
	node = f;
}

void decl_eraser::visit(stmt_block::Ptr block) {
	std::vector<stmt::Ptr> new_stmts;
	for (auto s: block->stmts) {
		if (s == to_erase)
			continue;
		s->accept(this);
		new_stmts.push_back(s);
	}
	block->stmts = new_stmts;	
}

void eliminate_redundant_vars(block::Ptr ast) {
	gather_redundant_decls gather;
	gather.ast = ast;
	ast->accept(&gather);
		
	while (gather.gathered_decls.size()) {
		decl_stmt::Ptr to_replace = gather.gathered_decls.back();
		gather.gathered_decls.pop_back();

		if (!isa<var_expr>(to_replace->init_expr)) {
			use_counter pre_counter;
			pre_counter.to_find = to_replace->decl_var;
			ast->accept(&pre_counter);
			
			if (pre_counter.total_uses != 1) {
				continue;
			}
		}
		
		replace_redundant_vars replacer;
		replacer.to_replace = to_replace;
		
		ast->accept(&replacer);

		// Now that replacing has been done
		// Count the number of occurences
		// If zero, delete
			
		use_counter counter;
		counter.to_find = to_replace->decl_var;
		ast->accept(&counter);

		if (counter.total_uses == 0) {
			decl_eraser eraser;
			eraser.to_erase = to_replace;	
			ast->accept(&eraser);	
		}	
	}
}


}

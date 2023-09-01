#include "blocks/block_visitor.h"
#include "blocks/stmt.h"

namespace block {
void block_visitor::visit(block::Ptr a) {}
void block_visitor::visit(expr::Ptr a) {}
void block_visitor::visit(unary_expr::Ptr a) {
	a->expr1->accept(this);
}
void block_visitor::visit(binary_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(not_expr::Ptr a) {
	a->expr1->accept(this);
}
void block_visitor::visit(and_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(bitwise_and_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(or_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(bitwise_or_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(plus_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(minus_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(mul_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(div_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(lt_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(gt_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(lte_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(gte_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(lshift_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(rshift_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(equals_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(ne_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(mod_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(var_expr::Ptr a) {
	a->var1->accept(this);
}
void block_visitor::visit(const_expr::Ptr a) {}
void block_visitor::visit(int_const::Ptr a) {}
void block_visitor::visit(double_const::Ptr a) {}
void block_visitor::visit(float_const::Ptr a) {}
void block_visitor::visit(string_const::Ptr a) {}
void block_visitor::visit(assign_expr::Ptr a) {
	a->var1->accept(this);
	a->expr1->accept(this);
}
void block_visitor::visit(stmt::Ptr a) {}
void block_visitor::visit(expr_stmt::Ptr a) {
	a->expr1->accept(this);
}
void block_visitor::visit(stmt_block::Ptr a) {
	for (auto stmt : a->stmts) {
		stmt->accept(this);
	}
}
void block_visitor::visit(decl_stmt::Ptr a) {
	a->decl_var->accept(this);
	if (a->init_expr != nullptr)
		a->init_expr->accept(this);
}
void block_visitor::visit(if_stmt::Ptr a) {
	a->cond->accept(this);
	a->then_stmt->accept(this);
	a->else_stmt->accept(this);
}
void block_visitor::visit(label::Ptr a) {}
void block_visitor::visit(label_stmt::Ptr a) {
	if (a->label1 != nullptr)
		a->label1->accept(this);
}
void block_visitor::visit(goto_stmt::Ptr a) {
	if (a->label1 != nullptr)
		a->label1->accept(this);
}
void block_visitor::visit(break_stmt::Ptr a) {}
void block_visitor::visit(continue_stmt::Ptr a) {}
void block_visitor::visit(while_stmt::Ptr a) {
	a->cond->accept(this);
	a->body->accept(this);
}
void block_visitor::visit(for_stmt::Ptr a) {
	a->decl_stmt->accept(this);
	a->cond->accept(this);
	a->update->accept(this);
	a->body->accept(this);
}
void block_visitor::visit(sq_bkt_expr::Ptr a) {
	a->var_expr->accept(this);
	a->index->accept(this);
}
void block_visitor::visit(function_call_expr::Ptr a) {
	a->expr1->accept(this);
	for (unsigned int i = 0; i < a->args.size(); i++) {
		a->args[i]->accept(this);
	}
}
void block_visitor::visit(initializer_list_expr::Ptr a) {
	for (unsigned int i = 0; i < a->elems.size(); i++) {
		a->elems[i]->accept(this);
	}
}
void block_visitor::visit(foreign_expr_base::Ptr a) {
	// Since this is an abstract class, we do nothing
}
void block_visitor::visit(var::Ptr a) {
	a->var_type->accept(this);
}
void block_visitor::visit(type::Ptr a) {}
void block_visitor::visit(scalar_type::Ptr a) {}
void block_visitor::visit(pointer_type::Ptr a) {
	a->pointee_type->accept(this);
}
void block_visitor::visit(reference_type::Ptr a) {
	a->referenced_type->accept(this);
}
void block_visitor::visit(function_type::Ptr a) {
	a->return_type->accept(this);
	for (unsigned int i = 0; i < a->arg_types.size(); i++)
		a->arg_types[i]->accept(this);
}
void block_visitor::visit(array_type::Ptr a) {
	a->element_type->accept(this);
}
void block_visitor::visit(builder_var_type::Ptr a) {
	a->closure_type->accept(this);
}
void block_visitor::visit(named_type::Ptr x) {
	for (auto a : x->template_args) {
		a->accept(this);
	}
}
void block_visitor::visit(func_decl::Ptr a) {
	a->return_type->accept(this);
	for (auto arg : a->args)
		arg->accept(this);
	a->body->accept(this);
}
void block_visitor::visit(return_stmt::Ptr a) {
	a->return_val->accept(this);
}

void block_visitor::visit(member_access_expr::Ptr a) {
	a->parent_expr->accept(this);
}
void block_visitor::visit(addr_of_expr::Ptr a) {
	a->expr1->accept(this);
}

} // namespace block

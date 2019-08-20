#include "blocks/block_visitor.h"
#include "blocks/stmt.h"

namespace block {
void block_visitor::visit(block::Ptr a) {
	
}
void block_visitor::visit(expr::Ptr a) {
}
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
void block_visitor::visit(or_expr::Ptr a) {
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
void block_visitor::visit(equals_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(ne_expr::Ptr a) {
	a->expr1->accept(this);
	a->expr2->accept(this);
}
void block_visitor::visit(var_expr::Ptr a) {
	a->var1->accept(this);
}
void block_visitor::visit(const_expr::Ptr a) {

}
void block_visitor::visit(int_const::Ptr a) {
}
void block_visitor::visit(assign_expr::Ptr a) {
	a->var1->accept(this);
	a->expr1->accept(this);
}
void block_visitor::visit(stmt::Ptr a) {
}
void block_visitor::visit(expr_stmt::Ptr a) {
	a->expr1->accept(this);
}
void block_visitor::visit(stmt_block::Ptr a) {
	for (auto stmt: a->stmts) {
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
void block_visitor::visit(label::Ptr a) {
	
}
void block_visitor::visit(label_stmt::Ptr a) {
	if (a->label1 != nullptr)		
		a->label1->accept(this);
}
void block_visitor::visit(goto_stmt::Ptr a) {
	if (a->label1 != nullptr)
		a->label1->accept(this);
}
void block_visitor::visit(break_stmt::Ptr a) {
}
void block_visitor::visit(while_stmt::Ptr a) {
	a->cond->accept(this);
	a->body->accept(this);
}
void block_visitor::visit(var::Ptr a) {
	a->var_type->accept(this);
}
void block_visitor::visit(type::Ptr a) {
}
void block_visitor::visit(scalar_type::Ptr a) {
	
}
void block_visitor::visit(pointer_type::Ptr a) {
	a->pointee_type->accept(this);
}
}


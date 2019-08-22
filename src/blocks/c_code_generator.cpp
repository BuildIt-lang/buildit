#include "blocks/c_code_generator.h"

namespace block {
void c_code_generator::visit(not_expr::Ptr a) {
	oss << "!(";
	a->expr1->accept(this);
	oss << ")";
}

static bool expr_needs_bracket(expr::Ptr a) {
	if (isa<binary_expr>(a))
		return true;
	else if (isa<assign_expr>(a))
		return true;
	return false;
}
void c_code_generator::emit_binary_expr(binary_expr::Ptr a, std::string character) {
	if (expr_needs_bracket(a->expr1)) {
		oss << "(";
		a->expr1->accept(this);
		oss << ")";
	} else 
		a->expr1->accept(this);
	oss << " " << character << " ";
	if (expr_needs_bracket(a->expr2)) {
		oss << "(";
		a->expr2->accept(this);
		oss << ")";
	} else
		a->expr2->accept(this);
}
void c_code_generator::visit(and_expr::Ptr a) {
	emit_binary_expr(a, "&&");
}
void c_code_generator::visit(or_expr::Ptr a) {
	emit_binary_expr(a, "||");
}
void c_code_generator::visit(plus_expr::Ptr a) {
	emit_binary_expr(a, "+");
}
void c_code_generator::visit(minus_expr::Ptr a) {
	emit_binary_expr(a, "-");
}
void c_code_generator::visit(mul_expr::Ptr a) {
	emit_binary_expr(a, "*");
}
void c_code_generator::visit(div_expr::Ptr a) {
	emit_binary_expr(a, "/");
}
void c_code_generator::visit(lt_expr::Ptr a) {
	emit_binary_expr(a, "<");
}
void c_code_generator::visit(gt_expr::Ptr a) {
	emit_binary_expr(a, ">");
}
void c_code_generator::visit(lte_expr::Ptr a) {
	emit_binary_expr(a, "<=");
}
void c_code_generator::visit(gte_expr::Ptr a) {
	emit_binary_expr(a, ">=");
}
void c_code_generator::visit(equals_expr::Ptr a) {
	emit_binary_expr(a, "==");
}
void c_code_generator::visit(ne_expr::Ptr a) {
	emit_binary_expr(a, "!=");
}
void c_code_generator::visit(var_expr::Ptr a) {
	oss << a->var1->var_name;
}
void c_code_generator::visit(int_const::Ptr a) {
	oss << a->value;
}
void c_code_generator::visit(assign_expr::Ptr a) {
	if (expr_needs_bracket(a->var1)) {
		oss << "(";
		a->var1->accept(this);
		oss << ")";
	} else 
		a->var1->accept(this);
	
	oss << " = ";
	a->expr1->accept(this);
}
void c_code_generator::visit(expr_stmt::Ptr a) {
	a->expr1->accept(this);
	oss << ";";
}
void c_code_generator::visit(stmt_block::Ptr a) {
	oss << "{" << std::endl;
	curr_indent += 1;
	for (auto stmt: a->stmts) {
		printer::indent(oss, curr_indent);
		stmt->accept(this);
		oss << std::endl;
	}
	curr_indent -= 1;
	printer::indent(oss, curr_indent);

	oss << "}";	
}
void c_code_generator::visit(scalar_type::Ptr type) {
	if (type->scalar_type_id == scalar_type::INT_TYPE) {
		oss << "int";
	} else if (type->scalar_type_id == scalar_type::VOID_TYPE) {
		oss << "void";
	}
}
void c_code_generator::visit(pointer_type::Ptr type) {
	type->pointee_type->accept(this);
	oss << "*";
}
void c_code_generator::visit(var::Ptr var) {
	oss << var->var_name;
}
void c_code_generator::visit(decl_stmt::Ptr a) {
	if (isa<function_type> (a->decl_var->var_type)) {
		function_type::Ptr type = to<function_type>(a->decl_var->var_type);
		type->return_type->accept(this);
		oss << " ";
		oss << a->decl_var->var_name;
		oss << "(";
		for (int i = 0; i < type->arg_types.size(); i++) {
			type->arg_types[i]->accept(this);
			if (i != type->arg_types.size()-1) 
				oss << ", ";
		}
		oss << ");";
		return;	
	}

	a->decl_var->var_type->accept(this);
	oss << " ";
	oss << a->decl_var->var_name;
	if (a->init_expr == nullptr) {
		oss << ";";
	} else {
		oss << " = ";
		a->init_expr->accept(this);
		oss << ";";
	}
}
void c_code_generator::visit(if_stmt::Ptr a) {
	oss << "if (";
	a->cond->accept(this);
	oss << ")";
	if (isa<stmt_block>(a->then_stmt)) {
		oss << " ";
		a->then_stmt->accept(this);
		oss << " ";
	} else {
		oss << std::endl;
		curr_indent++;
		printer::indent(oss, curr_indent);
		a->then_stmt->accept(this);
		oss << std::endl;
		curr_indent--;	
	}
	oss << "else";
	
	if (isa<stmt_block>(a->else_stmt)) {
		oss << " ";
		a->else_stmt->accept(this);
	} else {
		oss << std::endl;
		curr_indent++;
		printer::indent(oss, curr_indent);
		a->else_stmt->accept(this);
		curr_indent--;
	}
}
void c_code_generator::visit(while_stmt::Ptr a) {
	oss << "while (";
	a->cond->accept(this);
	oss << ")";
	if (isa<stmt_block>(a->body)) {
		oss << " ";
		a->body->accept(this);
	} else {
		oss << std::endl;
		curr_indent++;
		printer::indent(oss, curr_indent);
		a->body->accept(this);
		curr_indent--;
	}
}
void c_code_generator::visit(break_stmt::Ptr a) {
	oss << "break;";
}
void c_code_generator::visit(sq_bkt_expr::Ptr a) {
	if (expr_needs_bracket(a->var_expr)) {
		oss << "(";
	}
	a->var_expr->accept(this);
	if (expr_needs_bracket(a->var_expr)) {
		oss << ")";
	}
	oss << "[";
	a->index->accept(this);
	oss << "]";	
}
}

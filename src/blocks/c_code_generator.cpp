#include "blocks/c_code_generator.h"
#include <iomanip>
#include <limits>
#include <math.h>
#include <sstream>

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
void c_code_generator::visit(bitwise_and_expr::Ptr a) {
	emit_binary_expr(a, "&");
}
void c_code_generator::visit(or_expr::Ptr a) {
	emit_binary_expr(a, "||");
}
void c_code_generator::visit(bitwise_or_expr::Ptr a) {
	emit_binary_expr(a, "|");
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
void c_code_generator::visit(lshift_expr::Ptr a) {
	emit_binary_expr(a, "<<");
}
void c_code_generator::visit(rshift_expr::Ptr a) {
	emit_binary_expr(a, ">>");
}
void c_code_generator::visit(equals_expr::Ptr a) {
	emit_binary_expr(a, "==");
}
void c_code_generator::visit(ne_expr::Ptr a) {
	emit_binary_expr(a, "!=");
}
void c_code_generator::visit(mod_expr::Ptr a) {
	emit_binary_expr(a, "%");
}
void c_code_generator::visit(var_expr::Ptr a) {
	oss << a->var1->var_name;
}
void c_code_generator::visit(int_const::Ptr a) {
	oss << a->value;
	if (a->is_64bit)
		oss << "ll";
}
void c_code_generator::visit(double_const::Ptr a) {
	oss << std::setprecision(15);
	oss << a->value;
	if (floor(a->value) == a->value)
		oss << ".0";
}
void c_code_generator::visit(float_const::Ptr a) {
	oss << std::setprecision(15);
	oss << a->value;
	if (floor(a->value) == a->value)
		oss << ".0";
	oss << "f";
}
void c_code_generator::visit(string_const::Ptr a) {
	oss << "\"" << a->value << "\"";
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
	if (a->annotation != "")
		oss << " //" << a->annotation;
}
void c_code_generator::visit(stmt_block::Ptr a) {
	oss << "{" << std::endl;
	curr_indent += 1;
	for (auto stmt : a->stmts) {
		printer::indent(oss, curr_indent);
		stmt->accept(this);
		oss << std::endl;
	}
	curr_indent -= 1;
	printer::indent(oss, curr_indent);

	oss << "}";
}
void c_code_generator::visit(scalar_type::Ptr type) {
	switch (type->scalar_type_id) {
	case scalar_type::SHORT_INT_TYPE:
		oss << "short int";
		break;
	case scalar_type::UNSIGNED_SHORT_INT_TYPE:
		oss << "unsigned short int";
		break;
	case scalar_type::INT_TYPE:
		oss << "int";
		break;
	case scalar_type::UNSIGNED_INT_TYPE:
		oss << "unsigned int";
		break;
	case scalar_type::LONG_INT_TYPE:
		oss << "long int";
		break;
	case scalar_type::UNSIGNED_LONG_INT_TYPE:
		oss << "unsigned long int";
		break;
	case scalar_type::LONG_LONG_INT_TYPE:
		oss << "long long int";
		break;
	case scalar_type::UNSIGNED_LONG_LONG_INT_TYPE:
		oss << "unsigned long long int";
		break;
	case scalar_type::CHAR_TYPE:
		oss << "char";
		break;
	case scalar_type::UNSIGNED_CHAR_TYPE:
		oss << "unsigned char";
		break;
	case scalar_type::VOID_TYPE:
		oss << "void";
		break;
	case scalar_type::FLOAT_TYPE:
		oss << "float";
		break;
	case scalar_type::DOUBLE_TYPE:
		oss << "double";
		break;
	default:
		assert(false && "Invalid scalar type");
	}
}
void c_code_generator::visit(named_type::Ptr type) {
	oss << type->type_name;
	if (type->template_args.size()) {
		oss << "<";
		bool needs_comma = false;
		for (auto a : type->template_args) {
			if (needs_comma)
				oss << ", ";
			needs_comma = true;
			a->accept(this);
		}
		oss << ">";
	}
}
void c_code_generator::visit(pointer_type::Ptr type) {
	if (!isa<scalar_type>(type->pointee_type) && !isa<pointer_type>(type->pointee_type) &&
	    !isa<named_type>(type->pointee_type))
		assert(false && "Printing pointers of complex type is not supported yet");
	type->pointee_type->accept(this);
	oss << "*";
}
void c_code_generator::visit(reference_type::Ptr type) {
	if (!isa<scalar_type>(type->referenced_type) && !isa<pointer_type>(type->referenced_type) &&
	    !isa<named_type>(type->referenced_type))
		assert(false && "Printing pointers of complex type is not supported yet");
	type->referenced_type->accept(this);
	oss << "&";
}
void c_code_generator::visit(array_type::Ptr type) {
	if (!isa<scalar_type>(type->element_type) && !isa<pointer_type>(type->element_type) &&
	    !isa<named_type>(type->element_type))
		assert(false && "Printing arrays of complex type is not supported yet");
	type->element_type->accept(this);
	if (type->size != -1)
		oss << "[" << type->size << "]";
	else
		oss << "[]";
}
void c_code_generator::visit(builder_var_type::Ptr type) {
	if (type->builder_var_type_id == builder_var_type::DYN_VAR)
		oss << "builder::dyn_var<";
	else if (type->builder_var_type_id == builder_var_type::STATIC_VAR)
		oss << "builder::static_var<";
	type->closure_type->accept(this);
	oss << ">";
}
void c_code_generator::visit(var::Ptr var) {
	oss << var->var_name;
}

static void print_array_decl(std::ostream &oss, array_type::Ptr atype, var::Ptr decl_var, c_code_generator *self,
			     std::stringstream &append) {
	append << "[";
	if (atype->size != -1)
		append << atype->size;
	append << "]";

	if (isa<array_type>(atype->element_type))
		print_array_decl(oss, to<array_type>(atype->element_type), decl_var, self, append);
	else if (isa<scalar_type>(atype->element_type) || isa<pointer_type>(atype->element_type) ||
		 isa<named_type>(atype->element_type)) {
		atype->element_type->accept(self);
		if (decl_var->hasMetadata<std::vector<std::string>>("attributes")) {
			const auto &attributes = decl_var->getMetadata<std::vector<std::string>>("attributes");
			for (auto attr : attributes) {
				oss << " " << attr;
			}
		}
		oss << " ";
		oss << decl_var->var_name;
		oss << append.str();
	} else {
		assert(false && "Printing arrays of complex type is not supported yet");
	}
}

void c_code_generator::visit(decl_stmt::Ptr a) {
	if (isa<function_type>(a->decl_var->var_type)) {
		function_type::Ptr type = to<function_type>(a->decl_var->var_type);
		type->return_type->accept(this);
		oss << " (*";
		oss << a->decl_var->var_name;
		oss << ")(";
		for (unsigned int i = 0; i < type->arg_types.size(); i++) {
			type->arg_types[i]->accept(this);
			if (i != type->arg_types.size() - 1)
				oss << ", ";
		}
		oss << ")";
		if (a->init_expr != nullptr) {
			oss << " = ";
			a->init_expr->accept(this);
		}
		oss << ";";
		return;
	} else if (isa<array_type>(a->decl_var->var_type)) {
		array_type::Ptr type = to<array_type>(a->decl_var->var_type);
		std::stringstream s;
		print_array_decl(oss, type, a->decl_var, this, s);
		if (a->init_expr != nullptr) {
			oss << " = ";
			a->init_expr->accept(this);
		}
		oss << ";";
		return;
	}

	a->decl_var->var_type->accept(this);
	if (a->decl_var->hasMetadata<std::vector<std::string>>("attributes")) {
		const auto &attributes = a->decl_var->getMetadata<std::vector<std::string>>("attributes");
		for (auto attr : attributes) {
			oss << " " << attr;
		}
	}
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

	if (isa<stmt_block>(a->else_stmt)) {
		if (to<stmt_block>(a->else_stmt)->stmts.size() == 0)
			return;
		oss << "else";
		oss << " ";
		a->else_stmt->accept(this);
	} else {
		oss << "else";
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
void c_code_generator::visit(for_stmt::Ptr a) {
	oss << "for (";
	a->decl_stmt->accept(this);
	oss << " ";
	a->cond->accept(this);
	oss << "; ";
	a->update->accept(this);
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
void c_code_generator::visit(continue_stmt::Ptr a) {
	oss << "continue;";
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
void c_code_generator::visit(function_call_expr::Ptr a) {
	if (expr_needs_bracket(a->expr1)) {
		oss << "(";
	}
	a->expr1->accept(this);
	if (expr_needs_bracket(a->expr1)) {
		oss << ")";
	}
	oss << "(";
	for (unsigned int i = 0; i < a->args.size(); i++) {
		a->args[i]->accept(this);
		if (i != a->args.size() - 1)
			oss << ", ";
	}
	oss << ")";
}
void c_code_generator::visit(initializer_list_expr::Ptr a) {
	oss << "{";
	for (unsigned int i = 0; i < a->elems.size(); i++) {
		a->elems[i]->accept(this);
		if (i != a->elems.size() - 1)
			oss << ", ";
	}
	oss << "}";
}
void c_code_generator::handle_func_arg(var::Ptr a) {
	function_type::Ptr type = to<function_type>(a->var_type);
	type->return_type->accept(this);
	oss << " (*";
	oss << a->var_name;
	oss << ")(";
	for (unsigned int i = 0; i < type->arg_types.size(); i++) {
		type->arg_types[i]->accept(this);
		if (i != type->arg_types.size() - 1)
			oss << ", ";
	}
	oss << ")";
	return;
}
void c_code_generator::visit(func_decl::Ptr a) {
	a->return_type->accept(this);
	if (a->hasMetadata<std::vector<std::string>>("attributes")) {
		const auto &attributes = a->getMetadata<std::vector<std::string>>("attributes");
		for (auto attr : attributes) {
			oss << " " << attr;
		}
	}
	oss << " " << a->func_name;
	oss << " (";
	bool printDelim = false;
	for (auto arg : a->args) {
		if (printDelim)
			oss << ", ";
		printDelim = true;
		if (isa<function_type>(arg->var_type)) {
			handle_func_arg(arg);
		} else {
			arg->var_type->accept(this);
			oss << " " << arg->var_name;
		}
	}
	if (!printDelim)
		oss << "void";
	oss << ")";
	if (decl_only) {
		oss << ";";
		return;
	}
	if (isa<stmt_block>(a->body)) {
		oss << " ";
		a->body->accept(this);
		oss << std::endl;
	} else {
		oss << std::endl;
		curr_indent++;
		printer::indent(oss, curr_indent);
		a->body->accept(this);
		oss << std::endl;
		curr_indent--;
	}
}
void c_code_generator::visit(goto_stmt::Ptr a) {
	// a->dump(oss, 1);
	oss << "goto ";
	oss << a->label1->label_name << ";";
}
void c_code_generator::visit(label_stmt::Ptr a) {
	oss << a->label1->label_name << ":";
}
void c_code_generator::visit(return_stmt::Ptr a) {
	oss << "return ";
	a->return_val->accept(this);
	oss << ";";
}
void c_code_generator::visit(member_access_expr::Ptr a) {
	if (isa<sq_bkt_expr>(a->parent_expr)) {
		sq_bkt_expr::Ptr parent = to<sq_bkt_expr>(a->parent_expr);
		if (isa<int_const>(parent->index)) {
			auto index = to<int_const>(parent->index);
			if (index->value == 0) {
				if (!isa<var_expr>(parent->var_expr)) {
					oss << "(";
				}
				parent->var_expr->accept(this);
				if (!isa<var_expr>(parent->var_expr)) {
					oss << ")";
				}
				oss << "->" << a->member_name;
				return;
			}
		}
	}

	if (!isa<var_expr>(a->parent_expr))
		oss << "(";
	a->parent_expr->accept(this);
	if (!isa<var_expr>(a->parent_expr))
		oss << ")";

	oss << "." << a->member_name;
}
void c_code_generator::visit(addr_of_expr::Ptr a) {
	oss << "(&(";
	a->expr1->accept(this);
	oss << "))";
}
} // namespace block

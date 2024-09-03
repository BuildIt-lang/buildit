#include "blocks/expr.h"
#include "builder/builder_context.h"
#include "util/printer.h"
#include "util/tracer.h"

#include <execinfo.h>
#include <iostream>

namespace block {
// is_same implementation

bool expr::is_same(block::Ptr other) {
	if (static_offset != other->static_offset)
		return false;
	if (!isa<expr>(other))
		return false;
	return true;
}
// Empty implementation for abstract classes, can be changed to throw errors
// instead
void expr::dump(std::ostream &oss, int indent) {}

void unary_expr::dump(std::ostream &oss, int indent) {}

void binary_expr::dump(std::ostream &oss, int indent) {}
void const_expr::dump(std::ostream &oss, int indent) {}

void not_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "NOT_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
}
void unary_minus_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "UNARY_MINUS_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
}
void bitwise_not_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "BITWISE_NOT_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
}
void and_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "AND_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void bitwise_and_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "BITWISE_AND_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void or_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "OR_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void bitwise_or_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "BITWISE_OR_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void bitwise_xor_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "BITWISE_XOR_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void plus_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "PLUS_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void minus_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "MINUS_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void mul_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "MUL_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void div_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "DIV_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void lt_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "LT_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void gt_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "GT_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void lte_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "LTE_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void gte_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "GTE_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void lshift_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "LSHIFT_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void rshift_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "RSHIFT_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void equals_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "EQUALS_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void ne_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "NE_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void mod_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "MOD_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	expr2->dump(oss, indent + 1);
}
void assign_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "ASSIGN_EXPR" << std::endl;
	var1->dump(oss, indent + 1);
	expr1->dump(oss, indent + 1);
}
void sq_bkt_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "SQ_BKT_EXPR" << std::endl;
	var_expr->dump(oss, indent + 1);
	index->dump(oss, indent + 1);
}
void function_call_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "FUNCTION_CALL_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
	for (unsigned int i = 0; i < args.size(); i++)
		args[i]->dump(oss, indent + 1);
}
void initializer_list_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "INITIALIZER_LIST_EXPR" << std::endl;
	for (unsigned int i = 0; i < elems.size(); i++)
		elems[i]->dump(oss, indent + 1);
}
void var_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "VAR_EXPR" << std::endl;
	var1->dump(oss, indent + 1);
}

void int_const::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "INT_CONST (" << value << ")" << std::endl;
}
void double_const::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "DOUBLE_CONST (" << value << ")" << std::endl;
}
void float_const::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "FLOAT_CONST (" << value << ")" << std::endl;
}
void string_const::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "STRING_CONST (\"" << value << "\")" << std::endl;
}
void member_access_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "MEMBER_ACCESS_EXPR (" << member_name << ")" << std::endl;
	parent_expr->dump(oss, indent + 1);
}

void addr_of_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "ADDR_OF_EXPR" << std::endl;
	expr1->dump(oss, indent + 1);
}

} // namespace block

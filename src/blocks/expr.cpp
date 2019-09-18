#include "blocks/expr.h"
#include "builder/builder_context.h"
#include "util/printer.h"
#include "util/tracer.h"

#include <iostream>
#include <execinfo.h>

namespace block {
// is_same implementation

bool expr::is_same(block::Ptr other) {
	if (static_offset != other->static_offset)
		return false;
	if (!isa<expr>(other)) 
		return false;
	return true;
}
template <typename T>
bool unary_is_same(std::shared_ptr<T> first, block::Ptr other) {
	if (first->static_offset != other->static_offset)
		return false;
	if (!isa<unary_expr>(other))
		return false;
	typename T::Ptr other_expr = to<T>(other);
	if (!(first->expr1->is_same(other_expr->expr1)))
		return false;
	return true;	
	
}
template <typename T>
bool binary_is_same (std::shared_ptr<T> first, block::Ptr second) {
	if (first->static_offset != second->static_offset)
		return false;
	if (!isa<T>(second))
		return false;
	typename T::Ptr other_expr = to<T>(second);
	if (!(first->expr1->is_same(other_expr->expr1)))
		return false;
	if (!(first->expr2->is_same(other_expr->expr2)))
		return false;
	return true;
}






// Empty implementation for abstract classes, can be changed to throw errors instead
void expr::dump(std::ostream &oss, int indent) {
}

void unary_expr::dump(std::ostream &oss, int indent) {
}

void binary_expr::dump(std::ostream &oss, int indent) {
}
void const_expr::dump(std::ostream &oss, int indent) {
}

void not_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "NOT_EXPR" << std::endl;
	expr1->dump(oss, indent+1);	
}
void and_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "AND_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void or_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "OR_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void plus_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "PLUS_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void minus_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "MINUS_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void mul_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "MUL_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void div_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "DIV_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void lt_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "LT_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void gt_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "GT_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void lte_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "LTE_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void gte_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "GTE_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void equals_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "EQUALS_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void ne_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "NE_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void mod_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "MOD_EXPR" << std::endl;	
	expr1->dump(oss, indent+1);
	expr2->dump(oss, indent+1);
}
void assign_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "ASSIGN_EXPR" << std::endl;
	var1->dump(oss, indent+1);
	expr1->dump(oss, indent+1);
}
void sq_bkt_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "SQ_BKT_EXPR" << std::endl;
	var_expr->dump(oss, indent+1);
	index->dump(oss, indent+1);
}
void function_call_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "FUNCTION_CALL_EXPR" << std::endl;
	expr1->dump(oss, indent+1);
	for (unsigned int i = 0; i < args.size(); i++) 
		args[i]->dump(oss, indent+1);
}
void var_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "VAR_EXPR" << std::endl;
	var1->dump(oss, indent+1);
}

void int_const::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "INT_CONST (" << value << ")" << std::endl;
}

}


#include "blocks/expr.h"
#include "builder/builder_context.h"
#include "util/printer.h"

#include <iostream>

namespace block {

// Empty implementation for abstract classes, can be changed to throw errors instead
void expr::dump(std::ostream &oss, int indent) {
}

void unary_expr::dump(std::ostream &oss, int indent) {
}

void binary_expr::dump(std::ostream &oss, int indent) {
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
void var_expr::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "VAR_EXPR" << std::endl;
	var1->dump(oss, indent+1);
}

expr::Ptr operator && (const expr::Ptr &a, const expr::Ptr &b) {
	builder::builder_context * context = a->context;
	assert(context != nullptr);
	assert(context == b->context);
	
	and_expr::Ptr expr = std::make_shared<and_expr>();
	expr->context = context;
	
	context->remove_node_from_sequence(a);			
	context->remove_node_from_sequence(b);			
	
	expr->expr1 = a;
	expr->expr2 = b;
	
	context->add_node_to_sequence(expr);
	
	return expr;
}
}


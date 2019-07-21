#include "blocks/expr.h"
#include "builder/builder_context.h"
#include "util/printer.h"
#include "util/tracer.h"

#include <iostream>
#include <execinfo.h>

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


}


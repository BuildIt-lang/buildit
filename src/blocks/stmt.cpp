#include "blocks/stmt.h"
#include "util/printer.h"

namespace block{
// Empty definitions for abstract classes
void stmt::dump(std::ostream &oss, int indent) {
}
void expr_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "EXPR_STMT" << std::endl;
	expr1->dump(oss, indent+1);
}
void stmt_block::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "STMT_BLOCK" << std::endl;
	for (auto stmt: stmts) {
		stmt->dump(oss, indent+1);
	}
}
void decl_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "DECL_STMT" << std::endl;
	decl_var->dump(oss, indent+1);
	if (init_expr != nullptr)
		init_expr->dump(oss, indent+1);
	else {
		printer::indent(oss, indent+1);
		oss << "NO_INITIALIZATION" << std::endl;
	}	
}
void if_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "IF_STMT" << std::endl;
	cond->dump(oss, indent+1);
	then_stmt->dump(oss, indent+1);
	else_stmt->dump(oss, indent+1);		
}
}

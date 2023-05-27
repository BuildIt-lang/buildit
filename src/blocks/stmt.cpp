#include "blocks/stmt.h"
#include "util/printer.h"

namespace block {
// Empty definitions for abstract classes
void stmt::dump(std::ostream &oss, int indent) {}
void expr_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "EXPR_STMT" << std::endl;
	expr1->dump(oss, indent + 1);
}
void stmt_block::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "STMT_BLOCK" << std::endl;
	for (auto stmt : stmts) {
		stmt->dump(oss, indent + 1);
	}
}
void decl_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "DECL_STMT" << std::endl;
	decl_var->var_type->dump(oss, indent + 1);
	decl_var->dump(oss, indent + 1);
	if (init_expr != nullptr)
		init_expr->dump(oss, indent + 1);
	else {
		printer::indent(oss, indent + 1);
		oss << "NO_INITIALIZATION" << std::endl;
	}
}
void if_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "IF_STMT" << std::endl;
	cond->dump(oss, indent + 1);
	then_stmt->dump(oss, indent + 1);
	else_stmt->dump(oss, indent + 1);
}
void label_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "LABEL_STMT" << std::endl;
	if (label1 != nullptr)
		label1->dump(oss, indent + 1);
}
void goto_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	if (label1 != nullptr)
		oss << "GOTO_STMT" << std::endl;
	else
		oss << "GOTO_STMT (" << temporary_label_number.stringify() << ")" << std::endl;
	if (label1 != nullptr)
		label1->dump(oss, indent + 1);
}
void label::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "LABEL (" << label_name << ")" << std::endl;
}
void while_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "WHILE_STMT" << std::endl;
	cond->dump(oss, indent + 1);
	body->dump(oss, indent + 1);
}
void for_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "FOR_STMT" << std::endl;
	decl_stmt->dump(oss, indent + 1);
	cond->dump(oss, indent + 1);
	update->dump(oss, indent + 1);
	body->dump(oss, indent + 1);
}
void break_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "BREAK_STMT" << std::endl;
}
void continue_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "CONTINUE_STMT" << std::endl;
}

void func_decl::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "FUNC_DECL" << std::endl;
	return_type->dump(oss, indent + 1);
	for (auto arg : args) {
		arg->dump(oss, indent + 1);
	}
	body->dump(oss, indent + 1);
}
void return_stmt::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "RETURN_STMT" << std::endl;
	return_val->dump(oss, indent + 1);
}

} // namespace block

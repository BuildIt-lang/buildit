#include "blocks/annotation_finder.h"

namespace block {
void annotation_finder::visit(expr_stmt::Ptr stmt) {
	if (annotation_to_find == stmt->annotation) {
		found_stmt = stmt;
	}
}
void annotation_finder::visit(decl_stmt::Ptr stmt) {
	if (annotation_to_find == stmt->annotation) {
		found_stmt = stmt;
	}
}
void annotation_finder::visit(if_stmt::Ptr stmt) {
	if (annotation_to_find == stmt->annotation) {
		found_stmt = stmt;
		return;
	}
	stmt->then_stmt->accept(this);
	stmt->else_stmt->accept(this);
}
void annotation_finder::visit(while_stmt::Ptr stmt) {
	if (annotation_to_find == stmt->annotation) {
		found_stmt = stmt;
		return;
	}
	stmt->body->accept(this);
}
void annotation_finder::visit(for_stmt::Ptr stmt) {
	if (annotation_to_find == stmt->annotation) {
		found_stmt = stmt;
		return;
	}
	stmt->decl_stmt->accept(this);
	stmt->body->accept(this);
}
stmt::Ptr annotation_finder::find_annotation(block::Ptr ast, std::string label) {
	annotation_finder finder;
	finder.annotation_to_find = label;
	ast->accept(&finder);
	return finder.found_stmt;
}
} // namespace block

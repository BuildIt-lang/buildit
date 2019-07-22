#include "blocks/label_inserter.h"

namespace block {
void label_collector::visit(goto_stmt::Ptr a) {
	collected_labels.insert(a->temporary_label_number);
}
void label_creator::visit(stmt_block::Ptr a) {
	std::vector<stmt::Ptr> new_stmts;

	for (stmt::Ptr stmt: a->stmts) {
		if (collected_labels.count(stmt->static_offset) > 0) {
			label::Ptr new_label = std::make_shared<label>();
			new_label->static_offset = stmt->static_offset;
			new_label->label_name = "label" + std::to_string(stmt->static_offset);
			label_stmt::Ptr new_label_stmt = std::make_shared<label_stmt>();
			new_label_stmt->static_offset = -1;
			new_label_stmt->label1 = new_label;	
			new_stmts.push_back(new_label_stmt);
			collected_labels.erase(stmt->static_offset);
			offset_to_label[stmt->static_offset] = new_label;
		}
		new_stmts.push_back(stmt);
		stmt->accept(this);
	}
	a->stmts = new_stmts;
}
void label_inserter::visit(goto_stmt::Ptr a) {
	a->label1 = offset_to_label[a->temporary_label_number];
}
}

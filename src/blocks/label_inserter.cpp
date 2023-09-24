#include "blocks/label_inserter.h"
#include <algorithm>

namespace block {
void label_collector::visit(goto_stmt::Ptr a) {
	collected_labels.push_back(a->temporary_label_number);
}
void label_creator::visit(stmt_block::Ptr a) {
	std::vector<stmt::Ptr> new_stmts;

	for (stmt::Ptr stmt : a->stmts) {
		if (std::find(collected_labels.begin(), collected_labels.end(), stmt->static_offset) !=
		    collected_labels.end()) {
			label::Ptr new_label = std::make_shared<label>();
			new_label->static_offset = stmt->static_offset;
			// new_label->label_name =
			//"label" + stmt->static_offset.stringify();
			new_label->label_name = "label" + std::to_string(current_label);
			current_label++;
			label_stmt::Ptr new_label_stmt = std::make_shared<label_stmt>();
			new_label_stmt->static_offset.clear();
			new_label_stmt->label1 = new_label;
			new_stmts.push_back(new_label_stmt);

			offset_to_label[stmt->static_offset.stringify()] = new_label;
		}
		new_stmts.push_back(stmt);
		stmt->accept(this);
	}
	a->stmts = new_stmts;
}

void label_inserter::visit(label_stmt::Ptr a) {
	offset_to_label[a->label1->static_offset.stringify()] = a->label1;
}

void label_inserter::visit(goto_stmt::Ptr a) {
	// Pick any jump target with feature unstructured
	// otherwise pick the one that is in the parent block
	if (feature_unstructured)
		a->label1 = backup_offset_to_label[a->temporary_label_number.stringify()];
	else
		a->label1 = offset_to_label[a->temporary_label_number.stringify()];
}
} // namespace block

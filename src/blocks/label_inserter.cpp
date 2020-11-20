#include "blocks/label_inserter.h"
#include <algorithm>

namespace block {
void label_collector::visit(goto_stmt::Ptr a) {
	collected_labels.push_back(a->temporary_label_number);
}
static void erase_tag(std::vector<tracer::tag> &list, tracer::tag &erase) {
	std::vector<tracer::tag> new_list;
	for (unsigned int i = 0; i < list.size(); i++) {
		if (list[i] != erase) {
			new_list.push_back(list[i]);
		}
	}
	list = new_list;
}
void label_creator::visit(stmt_block::Ptr a) {
	std::vector<stmt::Ptr> new_stmts;

	for (stmt::Ptr stmt : a->stmts) {
		if (std::find(collected_labels.begin(), collected_labels.end(),
			      stmt->static_offset) != collected_labels.end()) {
			label::Ptr new_label = std::make_shared<label>();
			new_label->static_offset = stmt->static_offset;
			new_label->label_name =
			    "label" + stmt->static_offset.stringify();
			label_stmt::Ptr new_label_stmt =
			    std::make_shared<label_stmt>();
			new_label_stmt->static_offset.clear();
			new_label_stmt->label1 = new_label;
			new_stmts.push_back(new_label_stmt);
			// collected_labels.erase(stmt->static_offset);
			erase_tag(collected_labels, stmt->static_offset);
			offset_to_label[stmt->static_offset.stringify()] =
			    new_label;
		}
		new_stmts.push_back(stmt);
		stmt->accept(this);
	}
	a->stmts = new_stmts;
}
void label_inserter::visit(goto_stmt::Ptr a) {
	a->label1 = offset_to_label[a->temporary_label_number.stringify()];
}
} // namespace block

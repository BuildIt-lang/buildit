#ifndef LABEL_INSERTER_H
#define LABEL_INSERTER_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"
#include <unordered_map>
#include <unordered_set>

namespace block {
class label_collector : public block_visitor {
public:
	using block_visitor::visit;
	std::vector<tracer::tag> collected_labels;
	virtual void visit(goto_stmt::Ptr);
};
class label_creator : public block_visitor {
public:
	using block_visitor::visit;
	std::vector<tracer::tag> collected_labels;
	std::unordered_map<std::string, label::Ptr> offset_to_label;
	int current_label = 0;
	virtual void visit(stmt_block::Ptr);
};
class label_inserter : public block_visitor {
public:
	using block_visitor::visit;

	// The main table to hold static tag to label mapping
	// this table holds the jump target that is in the parent of the
	// jump statement
	std::unordered_map<std::string, label::Ptr> offset_to_label;
	// A backup table which has atleast one label that we can jump to
	// Only used when feature_unstructured is used
	std::unordered_map<std::string, label::Ptr> backup_offset_to_label;

	bool feature_unstructured;

	virtual void visit(goto_stmt::Ptr);
	virtual void visit(label_stmt::Ptr);
};
} // namespace block
#endif

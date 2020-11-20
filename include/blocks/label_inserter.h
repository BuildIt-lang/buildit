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
	virtual void visit(stmt_block::Ptr);
};
class label_inserter : public block_visitor {
public:
	using block_visitor::visit;
	std::unordered_map<std::string, label::Ptr> offset_to_label;
	virtual void visit(goto_stmt::Ptr);
};
} // namespace block
#endif

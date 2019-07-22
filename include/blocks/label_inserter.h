#ifndef LABEL_INSERTER_H
#define LABEL_INSERTER_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"
#include <unordered_set>
#include <unordered_map>

namespace block {
class label_collector: public block_visitor {
public:
	std::unordered_set<int32_t> collected_labels;	
	virtual void visit(goto_stmt::Ptr);

};
class label_creator: public block_visitor {
public:
	std::unordered_set<int32_t> collected_labels;
	std::unordered_map<int32_t, label::Ptr> offset_to_label;
	virtual void visit(stmt_block::Ptr);
		
};
class label_inserter: public block_visitor {
public:
	std::unordered_map<int32_t, label::Ptr> offset_to_label;
	virtual void visit(goto_stmt::Ptr);
};
}
#endif

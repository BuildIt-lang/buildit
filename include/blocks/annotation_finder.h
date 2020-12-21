#ifndef ANNOTATION_FINDER_H
#define ANNOTATION_FINDER_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"

namespace block {
class annotation_finder : public block_visitor {
public:
	using block_visitor::visit;
	std::string annotation_to_find;
	stmt::Ptr found_stmt = nullptr;
	virtual void visit(expr_stmt::Ptr);
	virtual void visit(decl_stmt::Ptr);
	virtual void visit(if_stmt::Ptr);
	virtual void visit(while_stmt::Ptr);
	virtual void visit(for_stmt::Ptr);
	static stmt::Ptr find_annotation(block::Ptr, std::string);
};
} // namespace block
#endif

#ifndef IF_SWITCHER_H
#define IF_SWITCHER_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"
namespace block {
class if_switcher : public block_visitor {
public:
	using block_visitor::visit;
	virtual void visit(if_stmt::Ptr);
};
} // namespace block
#endif

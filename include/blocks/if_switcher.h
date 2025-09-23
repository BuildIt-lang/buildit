#ifndef IF_SWITCHER_H
#define IF_SWITCHER_H
#include "blocks/block_replacer.h"
#include "blocks/stmt.h"
namespace block {
class if_switcher : public block_replacer {
public:
	using block_replacer::visit;
	virtual void visit(if_stmt::Ptr);
};
} // namespace block
#endif

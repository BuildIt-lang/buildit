#include "builder/builder_context.h"

namespace builder {
void builder_context::commit_uncommitted(void) {
	for (auto block_ptr: uncommitted_sequence) {
		block::expr_stmt::Ptr s = std::make_shared<block::expr_stmt>	();
		assert(block::isa<block::expr>(block_ptr));
		//s->expr1 = block::to<block::expr::Ptr>(block_ptr);
		assert(current_block_stmt != nullptr);
		current_block_stmt->stmts.push_back(s);			
	}	

}
void builder_context::remove_node_from_sequence(block::expr::Ptr e) {
	block::block::Ptr b = e;
	uncommitted_sequence.remove(b);
}
}

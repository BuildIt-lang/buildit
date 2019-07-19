#include "builder/builder_context.h"

namespace builder {
builder_context::builder_context() {
	current_block_stmt = std::make_shared<block::stmt_block>();
	ast = current_block_stmt;
}
void builder_context::commit_uncommitted(void) {
	for (auto block_ptr: uncommitted_sequence) {
		block::expr_stmt::Ptr s = std::make_shared<block::expr_stmt>();
		assert(block::isa<block::expr>(block_ptr));
		s->expr1 = block::to<block::expr>(block_ptr);
		assert(current_block_stmt != nullptr);
		current_block_stmt->stmts.push_back(s);			
	}	
	uncommitted_sequence.clear();

}
void builder_context::remove_node_from_sequence(block::expr::Ptr e) {
	uncommitted_sequence.remove(e);
}
void builder_context::add_node_to_sequence(block::expr::Ptr e) {
	uncommitted_sequence.push_back(e);
}
block::stmt::Ptr builder_context::extract_ast(void) {
	commit_uncommitted();
	return ast;
}
}

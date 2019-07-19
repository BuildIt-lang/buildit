#ifndef BUILDER_CONTEXT
#define BUILDER_CONTEXT
#include <list>
#include "blocks/expr.h"
#include "blocks/stmt.h"

namespace builder {
class builder_context {
public:

	std::list <block::block::Ptr> uncommitted_sequence;
	block::stmt::Ptr ast;
	block::stmt_block::Ptr current_block_stmt;


	builder_context();

	void commit_uncommitted(void);	
	void remove_node_from_sequence(block::expr::Ptr);
	void add_node_to_sequence(block::expr::Ptr);


	block::stmt::Ptr extract_ast(void);
	
};

}

#endif

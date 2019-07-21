#ifndef BUILDER_CONTEXT
#define BUILDER_CONTEXT
#include <list>
#include <vector>
#include "blocks/expr.h"
#include "blocks/stmt.h"

namespace builder {
class builder_context {
public:

	typedef void (*ast_function_type)(builder_context* context);

	
	std::list <block::block::Ptr> uncommitted_sequence;
	block::stmt::Ptr ast;
	block::stmt_block::Ptr current_block_stmt;	
	int var_name_counter;
	ast_function_type current_function;
	std::vector<bool> bool_vector;
		
	

	builder_context();

	void commit_uncommitted(void);	
	void remove_node_from_sequence(block::expr::Ptr);
	void add_node_to_sequence(block::expr::Ptr);


	block::stmt::Ptr extract_ast(void);
	block::stmt::Ptr extract_ast_from_function(ast_function_type, std::vector<bool> bl = std::vector<bool>());
		
};
bool get_next_bool_from_context(builder_context *context, block::expr::Ptr);

}

#endif

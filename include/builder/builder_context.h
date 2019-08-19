#ifndef BUILDER_CONTEXT
#define BUILDER_CONTEXT
#include <list>
#include <vector>
#include "blocks/expr.h"
#include "blocks/stmt.h"
#include <unordered_set>

namespace builder {
class builder;
class var;
class int_var;

class builder_context {
public:

	typedef void (*ast_function_type)();

	
	std::list <block::block::Ptr> uncommitted_sequence;
	block::stmt::Ptr ast;
	block::stmt_block::Ptr current_block_stmt;	
	ast_function_type current_function;
	std::vector<bool> bool_vector;
	std::unordered_set<int32_t> visited_offsets;
		
	builder_context();

	void commit_uncommitted(void);	
	void remove_node_from_sequence(block::expr::Ptr);
	void add_node_to_sequence(block::expr::Ptr);
	
	void add_stmt_to_current_block(block::stmt::Ptr);


	block::stmt::Ptr extract_ast(void);
	block::stmt::Ptr extract_ast_from_function(ast_function_type);
	block::stmt::Ptr extract_ast_from_function_internal(ast_function_type, std::vector<bool> bl = std::vector<bool>());
	
	std::string current_label;

private:
	static builder_context *current_builder_context;
	friend builder;
	friend var;
	friend int_var;
	friend void annotate(std::string);
	friend int32_t get_offset_in_function(ast_function_type _function);
};
bool get_next_bool_from_context(builder_context *context, block::expr::Ptr);
int32_t get_offset_in_function(builder_context::ast_function_type _function);

}

#endif

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
template <typename T>
class pointer_var;
template <typename T>
class static_var;
template <typename r_type, typename... a_types>
class function_var; 
template <typename... arg_types>
std::vector <block::expr::Ptr> extract_call_arguments_helper (const builder& first_arg, arg_types&... rest_args);

class tracking_tuple {
public:
        const unsigned char* ptr;
        uint32_t size;
        tracking_tuple (const unsigned char* _ptr, uint32_t _size): ptr(_ptr), size(_size) {}
	std::string snapshot(void) {
		std::string output_string;
		char temp[4];
		for (int i = 0; i < size; i++) {
			sprintf(temp, "%02x", ptr[i]);
			output_string += temp;	
		}
		return output_string;
	}
};

class builder_context {
public:

	typedef void (*ast_function_type)();

	
	std::list <block::block::Ptr> uncommitted_sequence;
	block::stmt::Ptr ast;
	block::stmt_block::Ptr current_block_stmt;	
	ast_function_type current_function;
	std::vector<bool> bool_vector;
	std::vector<tracer::tag> visited_offsets;
	bool is_visited_tag (tracer::tag &new_tag);
	void erase_tag(tracer::tag &erase_tag);
		
	builder_context();

	void commit_uncommitted(void);	
	void remove_node_from_sequence(block::expr::Ptr);
	void add_node_to_sequence(block::expr::Ptr);
	
	void add_stmt_to_current_block(block::stmt::Ptr);


	block::stmt::Ptr extract_ast(void);
	block::stmt::Ptr extract_ast_from_function(ast_function_type);
	block::stmt::Ptr extract_ast_from_function_internal(ast_function_type, std::vector<bool> bl = std::vector<bool>());
	
	std::string current_label;


	std::vector<tracking_tuple> static_var_tuples;
	
	std::vector<var*> assume_variables;
	
	template <typename T>
	T* assume_variable (std::string name) {
		T *new_asm_variable = new T(true);
		new_asm_variable->block_var->var_name = name;
		assume_variables.push_back(new_asm_variable);
		
		return new_asm_variable;	
	}	
	~builder_context();
private:
	static builder_context *current_builder_context;
	friend builder;
	friend var;
	friend int_var;

	template <typename T>
	friend class pointer_var;
	template <typename T>
	friend class static_var;
	template <typename r_type, typename... a_types>
	friend class function_var;

	template <typename... arg_types>
	friend std::vector <block::expr::Ptr> extract_call_arguments_helper (const builder& first_arg, arg_types&... rest_args);

	friend void annotate(std::string);
	friend tracer::tag get_offset_in_function(ast_function_type _function);
};
bool get_next_bool_from_context(builder_context *context, block::expr::Ptr);
tracer::tag get_offset_in_function(builder_context::ast_function_type _function);

}

#endif

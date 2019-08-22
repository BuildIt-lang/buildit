#ifndef BUILDER_H
#define BUILDER_H

#include <memory>
#include <string>
#include "blocks/var.h"
#include "builder/builder_context.h"
#include "builder/static_var.h"
#include <algorithm>

namespace builder {
// Builder objects are always alive only for duration of the RUN/SEQUENCE. 
// Never store pointers to these objects (across runs) or heap allocate them.
class var;
class builder {
public:
	builder() = default;	
	block::expr::Ptr block_expr;
	template <typename T>	
	builder builder_binary_op(const builder &);
	template <typename T>	
	builder builder_unary_op();
	builder operator && (const builder &);	
	builder operator || (const builder &);
	builder operator + (const builder &);
	builder operator - (const builder &);
	builder operator * (const builder &);
	builder operator / (const builder &);
	builder operator < (const builder &);
	builder operator > (const builder &);
	builder operator <= (const builder &);
	builder operator >= (const builder &);
	builder operator == (const builder &);
	builder operator != (const builder &);
	
	builder operator [] (const builder &);
	
	builder operator = (const builder &);
	
	builder operator ! ();
	explicit operator bool();

	builder (const int&);

	template<typename... types>	
	builder operator () (types&... args);
};
builder operator && (const int &a, const builder &);
builder operator || (const int &a, const builder &);
builder operator + (const int &a, const builder &);
builder operator - (const int &a, const builder &);
builder operator * (const int &a, const builder &);
builder operator / (const int &a, const builder &);

class var {
public:
	// Optional var name
	std::string var_name;
	block::var::Ptr block_var;
	block::decl_stmt::Ptr block_decl_stmt;

	static block::type::Ptr create_block_type(void) {
		// Cannot create block type for abstract class
		assert(false);
	}

	var() = default;
	
	operator builder() const;

	explicit operator bool();
	builder operator && (const builder &);
	builder operator || (const builder &);
	builder operator + (const builder &);
	builder operator - (const builder &);
	builder operator * (const builder &);
	builder operator / (const builder &);
	builder operator < (const builder &);
	builder operator > (const builder &);
	builder operator <= (const builder &);
	builder operator >= (const builder &);
	builder operator == (const builder &);
	builder operator != (const builder &);
	
	builder operator [] (const builder &);
	
	builder operator = (const builder &);

	builder operator ! ();

	template<typename... types>	
	builder operator () (types&... args) {
		return (operator builder()) (args...);
	}


	builder operator = (const var& a) {
		return operator builder() = a;
	}
};


class int_var: public var {
public:
	using var::operator = ; 
	static block::type::Ptr create_block_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::INT_TYPE;
		return type;
	}
	void create_int_var(void);
	int_var();
	int_var(const int_var&);
	int_var(const builder&);
	int_var(const int);
	builder operator = (const int_var& a) {
		return operator builder() = a;
	}
};
class void_var: public var {
public:
	static block::type::Ptr create_block_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::VOID_TYPE;
		return type;	
	}
private:
	void_var();
};
template <typename base_type> 
class pointer_var: public var {
public:
	using var::operator =;
	static block::type::Ptr create_block_type(void) {
		block::pointer_type::Ptr type = std::make_shared<block::pointer_type>();
		type->pointee_type = base_type::create_block_type();
		return type;
	}
	void create_pointer_var(void);
	pointer_var();
	pointer_var(const pointer_var&);
	pointer_var(const builder&);	
};
template <typename base_type>
void pointer_var<base_type>::create_pointer_var(void) {
	assert(builder_context::current_builder_context != nullptr);
	assert(builder_context::current_builder_context->current_block_stmt != nullptr);
	builder_context::current_builder_context->commit_uncommitted();
	block::var::Ptr pointer_var = std::make_shared<block::var>();	
	pointer_var->var_type = create_block_type();
	block_var = pointer_var;
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	block::decl_stmt::Ptr decl_stmt = std::make_shared<block::decl_stmt>();
	decl_stmt->static_offset = offset;
	decl_stmt->decl_var = pointer_var;
	decl_stmt->init_expr = nullptr;
	block_decl_stmt = decl_stmt;
	builder_context::current_builder_context->add_stmt_to_current_block(decl_stmt);
	pointer_var->static_offset = offset;
}	

template <typename base_type>
pointer_var<base_type>::pointer_var() {
	create_pointer_var();
}		
template <typename base_type>
pointer_var<base_type>::pointer_var(const pointer_var<base_type>& a): pointer_var<base_type>((builder)a) {
}
template <typename base_type>
pointer_var<base_type>::pointer_var(const builder& a) {
	builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);
	create_pointer_var();
	block_decl_stmt->init_expr = a.block_expr;	
}

template <typename... args>
std::vector<block::type::Ptr> extract_type_vector (void);

template <typename T, typename... args> 
std::vector<block::type::Ptr> extract_type_vector_helper (void) {
	std::vector<block::type::Ptr> rest = extract_type_vector<args...>();
	rest.push_back(T::create_block_type());
	return rest;
}

template <typename... args>
std::vector<block::type::Ptr> extract_type_vector (void) {
	return extract_type_vector_helper<args...>();
}

template <>
std::vector<block::type::Ptr> extract_type_vector<> (void);


template <typename r_type, typename... a_types> 
class function_var: public var {
public:
	using var::operator =;
	static block::type::Ptr create_block_type(void) {
		block::function_type::Ptr type = std::make_shared<block::function_type>();
		type->return_type = r_type::create_block_type();	
		type->arg_types = extract_type_vector<a_types...>();
		std::reverse(type->arg_types.begin(), type->arg_types.end());
		return type;
	}	
	void create_function_var(void);
	function_var();
};
template <typename r_type, typename... a_types>
void function_var<r_type, a_types...>::create_function_var(void) {
	assert(builder_context::current_builder_context != nullptr);
	assert(builder_context::current_builder_context->current_block_stmt != nullptr);
	builder_context::current_builder_context->commit_uncommitted();
	block::var::Ptr function_var = std::make_shared<block::var>();	
	function_var->var_type = create_block_type();
	block_var = function_var;
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	block::decl_stmt::Ptr decl_stmt = std::make_shared<block::decl_stmt>();
	decl_stmt->static_offset = offset;
	decl_stmt->decl_var = function_var;
	decl_stmt->init_expr = nullptr;
	block_decl_stmt = decl_stmt;
	builder_context::current_builder_context->add_stmt_to_current_block(decl_stmt);
	function_var->static_offset = offset;
}	
template <typename r_type, typename... a_types>
function_var<r_type, a_types...>::function_var() {
	create_function_var();
}		

void annotate(std::string);

// The implementation for () operator on builder
template <typename... arg_types>
std::vector <block::expr::Ptr> extract_call_arguments (arg_types&... args);

template <typename... arg_types>
std::vector <block::expr::Ptr> extract_call_arguments_helper (const builder& first_arg, arg_types&... rest_args) {
	assert(builder_context::current_builder_context != nullptr);
	builder_context::current_builder_context->remove_node_from_sequence(first_arg.block_expr);
		
	std::vector<block::expr::Ptr> rest = extract_call_arguments(rest_args...);
	rest.push_back(first_arg.block_expr);
	return rest;
}

template <typename... arg_types>
std::vector <block::expr::Ptr> extract_call_arguments (arg_types&... args) {
	return extract_call_arguments_helper(args...);
}

template <>
std::vector <block::expr::Ptr> extract_call_arguments<> (void);


template <typename ...arg_types>
builder builder::operator () (arg_types& ... args) {
	assert(builder_context::current_builder_context != nullptr);
	
	builder_context::current_builder_context->remove_node_from_sequence(block_expr);
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	
	block::function_call_expr::Ptr expr = std::make_shared<block::function_call_expr>();
	expr->static_offset = offset;
	
	expr->expr1 = block_expr;
	expr->args = extract_call_arguments (args...);
	std::reverse(expr->args.begin(), expr->args.end());
	builder_context::current_builder_context->add_node_to_sequence(expr);
	
	
	builder ret_builder;
	ret_builder.block_expr = expr;
	return ret_builder;
}

}
#endif

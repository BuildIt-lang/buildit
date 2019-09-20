#ifndef BUILDER_H
#define BUILDER_H

#include <memory>
#include <string>
#include "blocks/var.h"
#include "builder/builder_context.h"
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
	builder operator % (const builder &);
	
	builder operator [] (const builder &);
	
	builder operator = (const builder &);
	
	builder operator ! ();
	explicit operator bool();

	builder (const int&);

	template<typename... types>	
	builder operator () (const types&... args);
	static builder sentinel_builder;
};
builder operator && (const int &a, const builder &);
builder operator || (const int &a, const builder &);
builder operator + (const int &a, const builder &);
builder operator - (const int &a, const builder &);
builder operator * (const int &a, const builder &);
builder operator / (const int &a, const builder &);

builder operator < (const int &a, const builder &);
builder operator > (const int &a, const builder &);
builder operator <= (const int &a, const builder &);
builder operator >= (const int &a, const builder &);
builder operator == (const int &a, const builder &);
builder operator != (const int &a, const builder &);
builder operator % (const int &a, const builder &);

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
	builder operator % (const builder &);
	
	builder operator [] (const builder &);
	
	builder operator = (const builder &);

	builder operator ! ();

	template<typename... types>	
	builder operator () (const types&... args) {
		return (operator builder()) (args...);
	}

	builder operator = (const var& a) {
		return operator builder() = a;
	}

	virtual ~var() = default;
};

template <typename T>
class type_extractor {
public:
	static block::type::Ptr extract_type(void);
};

template <>
class type_extractor<int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::INT_TYPE;
		return type;
	}
};

template <typename T>
class type_extractor<T*> {
public:
	static block::type::Ptr extract_type(void) {
		block::pointer_type::Ptr type = std::make_shared<block::pointer_type>();
		type->pointee_type = type_extractor<T>::extract_type();
		return type;
	}
};
template <>
class type_extractor<void> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::VOID_TYPE;
		return type;	
	}
};

template <typename T, int x>
class type_extractor<T[x]> {
public:
	static block::type::Ptr extract_type(void) {
		block::array_type::Ptr type = std::make_shared<block::array_type>();
		type->element_type = type_extractor<T>::extract_type();
		type->size = x;
		return type;
	}
};

template <typename T>
class type_extractor<T[]> {
public:
	static block::type::Ptr extract_type(void) {
		block::array_type::Ptr type = block::to<block::array_type>(type_extractor<T[1]>::extract_type());
		type->size = -1;
		return type;
	}
};

template <typename... args>
std::vector<block::type::Ptr> extract_type_vector_dyn(void);

template <typename T, typename... args> 
std::vector<block::type::Ptr> extract_type_vector_helper_dyn(void) {
	std::vector<block::type::Ptr> rest = extract_type_vector_dyn<args...>();
	rest.push_back(type_extractor<T>::extract_type());
	return rest;
}

template <typename... args>
std::vector<block::type::Ptr> extract_type_vector_dyn(void) {
	return extract_type_vector_helper_dyn<args...>();
}

template <>
std::vector<block::type::Ptr> extract_type_vector_dyn<> (void);

template <typename r_type, typename... a_types>
class type_extractor<r_type (a_types...)> {
public:
	static block::type::Ptr extract_type(void) {
		block::function_type::Ptr type = std::make_shared<block::function_type>();
		type->return_type = type_extractor<r_type>::extract_type();
		type->arg_types = extract_type_vector_dyn<a_types...>();
		std::reverse(type->arg_types.begin(), type->arg_types.end());
		return type;	
	}
};

template <typename T>
class dyn_var: public var{
public:
	using var::operator = ; 
	static block::type::Ptr create_block_type(void) {
		return type_extractor<T>::extract_type();	
	}	
	void create_dyn_var(bool create_without_context = false) {
		if (create_without_context) {
			block::var::Ptr dyn_var = std::make_shared<block::var>();	
			dyn_var->var_type = create_block_type();
			block_var = dyn_var;
			return;
		}
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->current_block_stmt != nullptr);
		builder_context::current_builder_context->commit_uncommitted();
		block::var::Ptr dyn_var = std::make_shared<block::var>();	
		dyn_var->var_type = create_block_type();
		block_var = dyn_var;
		tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
		dyn_var->static_offset = offset;
		block_decl_stmt = nullptr;
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block::decl_stmt::Ptr decl_stmt = std::make_shared<block::decl_stmt>();
		decl_stmt->static_offset = offset;
		decl_stmt->decl_var = dyn_var;
		decl_stmt->init_expr = nullptr;
		block_decl_stmt = decl_stmt;
		builder_context::current_builder_context->add_stmt_to_current_block(decl_stmt);
	}
	dyn_var(bool create_without_context=false) {
		create_dyn_var(create_without_context);
	}		
	dyn_var(const dyn_var<T>& a): dyn_var<T>((builder)a) {
	}
	template <typename TO>
	dyn_var(const dyn_var<TO>& a): dyn_var<TO>((builder)a) {
	}
	dyn_var(const builder& a) {
		builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);
		create_dyn_var();
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block_decl_stmt->init_expr = a.block_expr;	
	}
	dyn_var(const int& a): dyn_var((builder)a) {
	}
	virtual ~dyn_var() = default;

	template <typename TO>
	builder operator = (const dyn_var<TO>& a) {
		return operator builder() = a;
	}
	builder operator = (const int &a) {
		return operator = ((builder)a);
	}
};

void annotate(std::string);

// The implementation for () operator on builder
template <typename... arg_types>
std::vector <block::expr::Ptr> extract_call_arguments (const arg_types&... args);

template <typename... arg_types>
std::vector <block::expr::Ptr> extract_call_arguments_helper (const builder& first_arg, const arg_types&... rest_args) {
	assert(builder_context::current_builder_context != nullptr);
	builder_context::current_builder_context->remove_node_from_sequence(first_arg.block_expr);
		
	std::vector<block::expr::Ptr> rest = extract_call_arguments(rest_args...);
	rest.push_back(first_arg.block_expr);
	return rest;
}

template <typename... arg_types>
std::vector <block::expr::Ptr> extract_call_arguments (const arg_types&... args) {
	return extract_call_arguments_helper(args...);
}

template <>
std::vector <block::expr::Ptr> extract_call_arguments<> (void);


template <typename ...arg_types>
builder builder::operator () (const arg_types& ... args) {
	assert(builder_context::current_builder_context != nullptr);
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return builder::sentinel_builder;
	
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

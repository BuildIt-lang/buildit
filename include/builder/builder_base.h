#ifndef BUILDER_BASE_H
#define BUILDER_BASE_H

#include "builder/forward_declarations.h"

#include "blocks/var.h"
#include "builder/member_base.h"
#include "builder/builder_context.h"
#include "builder/signature_extract.h"
#include "builder/block_type_extractor.h"
#include <algorithm>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>


namespace builder {

// Builder objects are always alive only for duration of the RUN/SEQUENCE.
// Never store pointers to these objects (across runs) or heap allocate them.

template <typename... arg_types>
std::vector<block::expr::Ptr> extract_call_arguments(const arg_types &... args);
template <typename... arg_types>
std::vector<block::expr::Ptr> extract_call_arguments_helper(const arg_types &... args);



template <typename BT>
class builder_base {
public:

// All members here
	block::expr::Ptr block_expr;
	static BT sentinel_builder;
	
	typedef builder_base<BT> super;	

// All the costructors and copy constructors to the top

	// Simple constrcutor, should only be used inside the operator
	// and set the block_expr immediately
	builder_base() = default;
	// Copy constructor from another builder
	builder_base(const BT& other) {
		block_expr = other.block_expr;
	}

	template <typename T>	
	builder_base(const T& a, typename std::enable_if<std::is_base_of<member_base, T>::value && !std::is_base_of<var, T>::value, int>::type = 0) {
		block_expr = a.get_parent();
	}
	
	builder_base(const int &a) {
		assert(builder_context::current_builder_context != nullptr);
		block_expr = nullptr;
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block::int_const::Ptr int_const = std::make_shared<block::int_const>();
		tracer::tag offset = get_offset_in_function();
		int_const->static_offset = offset;
		int_const->value = a;
		builder_context::current_builder_context->add_node_to_sequence(
		    int_const);
		block_expr = int_const;
	}

	builder_base(const double &a) {
		assert(builder_context::current_builder_context != nullptr);
		block_expr = nullptr;
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block::double_const::Ptr double_const =
		    std::make_shared<block::double_const>();
		tracer::tag offset = get_offset_in_function();
		double_const->static_offset = offset;
		double_const->value = a;
		builder_context::current_builder_context->add_node_to_sequence(
		    double_const);

		block_expr = double_const;
	}
	builder_base(const float &a) {
		assert(builder_context::current_builder_context != nullptr);
		block_expr = nullptr;
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block::float_const::Ptr float_const =
		    std::make_shared<block::float_const>();
		tracer::tag offset = get_offset_in_function();
		float_const->static_offset = offset;
		float_const->value = a;
		builder_context::current_builder_context->add_node_to_sequence(
		    float_const);

		block_expr = float_const;
	}
	builder_base(const bool &b) : builder_base<BT>((int)b) {}
	builder_base(const char &c) : builder_base<BT>((int)c) {}
	builder_base(unsigned char &c) : builder_base<BT>((int)c) {}

	// This is a template class declaration but requires access to var
	// So this is defined after the var class definition
	builder_base(const var &a);


	template <typename T>
	builder_base(const static_var<T> &a) : builder_base<BT>((const T)a) {}


	bool builder_precheck(void) const {
		assert(builder_context::current_builder_context != nullptr);
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return true;
		return false;
	}

// Other basic functions	
	template <typename T>
	BT builder_unary_op() const {
		if (builder_precheck())
			return sentinel_builder;

		builder_context::current_builder_context->remove_node_from_sequence(
		    block_expr);
		tracer::tag offset = get_offset_in_function();

		typename T::Ptr expr = std::make_shared<T>();
		expr->static_offset = offset;
		expr->expr1 = block_expr;
		builder_context::current_builder_context->add_node_to_sequence(expr);

		BT ret_builder;
		ret_builder.block_expr = expr;
		return ret_builder;
	}


	template <typename T>
	BT builder_binary_op(const builder_base<BT> & a) const {
		if (builder_precheck())
			return sentinel_builder;

		builder_context::current_builder_context->remove_node_from_sequence(
		    block_expr);
		builder_context::current_builder_context->remove_node_from_sequence(
		    a.block_expr);

		tracer::tag offset = get_offset_in_function();

		typename T::Ptr expr = std::make_shared<T>();
		expr->static_offset = offset;

		expr->expr1 = block_expr;
		expr->expr2 = a.block_expr;

		builder_context::current_builder_context->add_node_to_sequence(expr);

		BT ret_builder;
		ret_builder.block_expr = expr;
		return ret_builder;
	}
	

	BT operator[](const BT& a) {
		if (builder_precheck())
			return sentinel_builder;
		builder_context::current_builder_context->remove_node_from_sequence(
		    block_expr);
		builder_context::current_builder_context->remove_node_from_sequence(
		    a.block_expr);

		tracer::tag offset = get_offset_in_function();
		// assert(offset != -1);

		block::sq_bkt_expr::Ptr expr = std::make_shared<block::sq_bkt_expr>();
		expr->static_offset = offset;

		expr->var_expr = block_expr;
		expr->index = a.block_expr;

		builder_context::current_builder_context->add_node_to_sequence(expr);

		BT ret_builder;
		ret_builder.block_expr = expr;
		return ret_builder;
	}

	BT assign(const BT& a) {
		if (builder_precheck())
			return sentinel_builder;

		builder_context::current_builder_context->remove_node_from_sequence(
		    block_expr);
		builder_context::current_builder_context->remove_node_from_sequence(
		    a.block_expr);
		tracer::tag offset = get_offset_in_function();
		// assert(offset != -1);

		block::assign_expr::Ptr expr = std::make_shared<block::assign_expr>();
		expr->static_offset = offset;

		expr->var1 = block_expr;
		expr->expr1 = a.block_expr;

		builder_context::current_builder_context->add_node_to_sequence(expr);

		BT ret_builder;
		ret_builder.block_expr = expr;
		return ret_builder;
	}
	BT operator=(const BT &a) {
		return assign(a);	
	}

	explicit operator bool() {
		builder_context::current_builder_context->commit_uncommitted();
		return get_next_bool_from_context(builder_context::current_builder_context, block_expr);
	}

	template <typename... arg_types>
	BT operator()(const arg_types &... args) {
		if (builder_precheck())
			return sentinel_builder;

		builder_context::current_builder_context->remove_node_from_sequence(block_expr);
		tracer::tag offset = get_offset_in_function();

		block::function_call_expr::Ptr expr = std::make_shared<block::function_call_expr>();
		expr->static_offset = offset;

		expr->expr1 = block_expr;
		expr->args = extract_call_arguments(args...);
		std::reverse(expr->args.begin(), expr->args.end());
		builder_context::current_builder_context->add_node_to_sequence(expr);

		BT ret_builder;
		ret_builder.block_expr = expr;
		return ret_builder;
	}


	template <typename T>
	void construct_builder_from_foreign_expr(const T &t) {
		assert(builder_context::current_builder_context != nullptr);
		block_expr = nullptr;
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block_expr = create_foreign_expr(t);
	}

};

template <typename BT>
BT builder_base<BT>::sentinel_builder;



void annotate(std::string);

// Helper function for the implementation of () operator on builder_base
template <typename BT, typename... arg_types>
typename std::enable_if<std::is_base_of<builder_base<BT>, BT>::value, std::vector<block::expr::Ptr>>::type extract_call_arguments_helper(const BT &first_arg, const arg_types &... rest_args) {
	assert(builder_context::current_builder_context != nullptr);
	builder_context::current_builder_context->remove_node_from_sequence(first_arg.block_expr);

	std::vector<block::expr::Ptr> rest = extract_call_arguments_helper(rest_args...);
	rest.push_back(first_arg.block_expr);
	return rest;
}

template <typename T, typename... arg_types>
std::vector<block::expr::Ptr> extract_call_arguments_helper(const dyn_var<T>& first_arg, const arg_types &... rest_args) {
	return extract_call_arguments_helper((builder)first_arg, rest_args...);
}

template <typename... arg_types>
std::vector<block::expr::Ptr> extract_call_arguments(const arg_types &... args) {
	return extract_call_arguments_helper(args...);
}

// Helper functions to create foreign expressions of arbitrary types
// Theses should be only called from the function to cast the type to builder_base classes
template <typename T>
block::expr::Ptr create_foreign_expr(const T t) {
	assert(builder_context::current_builder_context != nullptr);
	tracer::tag offset = get_offset_in_function();
	typename block::foreign_expr<T>::Ptr expr = std::make_shared<block::foreign_expr<T>>();
	expr->static_offset = offset;
	expr->inner_expr = t;
	builder_context::current_builder_context->add_node_to_sequence(expr);
	return expr;
}

template <typename BT, typename T>
BT create_foreign_expr_builder(const T t) {
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return BT::sentinel_builder;
	BT ret_builder;
	ret_builder.block_expr = create_foreign_expr(t);
	return ret_builder;
}


} // namespace builder
#endif

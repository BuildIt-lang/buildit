#ifndef BUILDER_BASE_H
#define BUILDER_BASE_H

#include "builder/forward_declarations.h"

#include "blocks/var.h"
#include "builder/block_type_extractor.h"
#include "builder/builder_context.h"
#include "builder/signature_extract.h"
#include <algorithm>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>

namespace builder {

// Builder objects are always alive only for duration of the RUN/SEQUENCE.
// Never store pointers to these objects (across runs) or heap allocate them.

template <typename BT, typename... arg_types>
std::vector<block::expr::Ptr> extract_call_arguments(const arg_types &...args);

template <typename BT, typename... arg_types>
std::vector<block::expr::Ptr> extract_call_arguments_helper(const arg_types &...args);

class builder {

public:
	// All members here
	block::expr::Ptr block_expr;

	// All the costructors and copy constructors to the top
	// Simple constrcutor, should only be used inside the operator
	// and set the block_expr immediately
	builder() = default;
	// Copy constructor from another builder
	builder(const builder &other) {
		block_expr = other.block_expr;
	}

	static bool builder_precheck(void) {
		return get_run_state()->is_catching_up();
	}
	void builder_from_sequence(void) {
		block_expr = get_run_state()->get_next_cached_expr();
	}
	static builder create_builder_from_sequence(void) {
		builder ret_builder;
		ret_builder.builder_from_sequence();
		return ret_builder;
	}
	static void push_to_sequence(block::expr::Ptr a) {
		get_run_state()->add_to_cached_expr(a);
	}
	builder(const unsigned int &a) : builder((int)a) {}

	builder(const int &a) {
		if (builder_precheck()) {
			builder_from_sequence();
			return;
		}

		block::int_const::Ptr int_const = std::make_shared<block::int_const>();
		tracer::tag offset = tracer::get_offset_in_function();
		int_const->static_offset = offset;
		int_const->value = a;
		int_const->is_64bit = false;
		get_run_state()->add_node_to_sequence(int_const);
		block_expr = int_const;

		push_to_sequence(block_expr);
	}
	builder(const unsigned long long &a) : builder((long long)a) {}
	builder(const long long &a) {
		if (builder_precheck()) {
			builder_from_sequence();
			return;
		}

		block::int_const::Ptr int_const = std::make_shared<block::int_const>();
		tracer::tag offset = tracer::get_offset_in_function();
		int_const->static_offset = offset;
		int_const->value = a;
		int_const->is_64bit = true;
		get_run_state()->add_node_to_sequence(int_const);
		block_expr = int_const;

		push_to_sequence(block_expr);
	}

	builder(const unsigned long &a): builder((unsigned long long)a){}
	builder(const long &a): builder((long long)a){}

	builder(const double &a) {
		if (builder_precheck()) {
			builder_from_sequence();
			return;
		}

		block::double_const::Ptr double_const = std::make_shared<block::double_const>();
		tracer::tag offset = tracer::get_offset_in_function();
		double_const->static_offset = offset;
		double_const->value = a;
		get_run_state()->add_node_to_sequence(double_const);
		block_expr = double_const;

		push_to_sequence(block_expr);
	}
	builder(const float &a) {
		if (builder_precheck()) {
			builder_from_sequence();
			return;
		}

		block::float_const::Ptr float_const = std::make_shared<block::float_const>();
		tracer::tag offset = tracer::get_offset_in_function();
		float_const->static_offset = offset;
		float_const->value = a;
		get_run_state()->add_node_to_sequence(float_const);
		block_expr = float_const;

		push_to_sequence(block_expr);
	}
	builder(const bool &b) : builder((int)b) {}
	builder(const char &c) : builder((int)c) {}
	builder(unsigned char &c) : builder((int)c) {}
	builder(const std::string &s) {
		if (builder_precheck()) {
			builder_from_sequence();
			return;
		}

		block::string_const::Ptr string_const = std::make_shared<block::string_const>();
		tracer::tag offset = tracer::get_offset_in_function();
		string_const->static_offset = offset;
		string_const->value = s;
		get_run_state()->add_node_to_sequence(string_const);
		block_expr = string_const;

		push_to_sequence(block_expr);
	}
	builder(const char *s) : builder((std::string)s) {}
	builder(char *s) : builder((std::string)s) {}

	// This is a template class declaration but requires access to var
	// So this is defined after the var class definition
	builder(const var &a);

	template <typename T>
	builder(const static_var<T> &a) : builder((const T)a) {}

	// Other basic functions
	template <typename T>
	builder builder_unary_op() const {
		if (builder_precheck()) {
			return create_builder_from_sequence();
		}
		get_run_state()->remove_node_from_sequence(block_expr);
		tracer::tag offset = tracer::get_offset_in_function();

		typename T::Ptr expr = std::make_shared<T>();
		expr->static_offset = offset;
		expr->expr1 = block_expr;
		get_run_state()->add_node_to_sequence(expr);

		builder ret_builder;
		ret_builder.block_expr = expr;
		push_to_sequence(expr);
		return ret_builder;
	}

	template <typename T>
	builder builder_binary_op(const builder &a) const {
		if (builder_precheck()) {
			return create_builder_from_sequence();
		}
		get_run_state()->remove_node_from_sequence(block_expr);
		get_run_state()->remove_node_from_sequence(a.block_expr);

		tracer::tag offset = tracer::get_offset_in_function();

		typename T::Ptr expr = std::make_shared<T>();
		expr->static_offset = offset;

		expr->expr1 = block_expr;
		expr->expr2 = a.block_expr;

		get_run_state()->add_node_to_sequence(expr);

		builder ret_builder;
		ret_builder.block_expr = expr;
		push_to_sequence(expr);
		return ret_builder;
	}

	builder operator[](const builder &a) {
		if (builder_precheck()) {
			return create_builder_from_sequence();
		}
		get_run_state()->remove_node_from_sequence(block_expr);
		get_run_state()->remove_node_from_sequence(a.block_expr);

		tracer::tag offset = tracer::get_offset_in_function();
		// assert(offset != -1);

		block::sq_bkt_expr::Ptr expr = std::make_shared<block::sq_bkt_expr>();
		expr->static_offset = offset;

		expr->var_expr = block_expr;
		expr->index = a.block_expr;

		get_run_state()->add_node_to_sequence(expr);

		builder ret_builder;
		ret_builder.block_expr = expr;
		push_to_sequence(expr);
		return ret_builder;
	}
	builder operator*(void) {
		auto b = (*this)[0];
		b.block_expr->template setMetadata<bool>("deref_is_star", true);
		return b;
	}

	builder assign(const builder &a) {
		if (builder_precheck()) {
			return create_builder_from_sequence();
		}

		get_run_state()->remove_node_from_sequence(block_expr);
		get_run_state()->remove_node_from_sequence(a.block_expr);
		tracer::tag offset = tracer::get_offset_in_function();

		block::assign_expr::Ptr expr = std::make_shared<block::assign_expr>();
		expr->static_offset = offset;

		expr->var1 = block_expr;
		expr->expr1 = a.block_expr;

		get_run_state()->add_node_to_sequence(expr);

		builder ret_builder;
		ret_builder.block_expr = expr;
		push_to_sequence(expr);
		return ret_builder;
	}
	builder operator=(const builder &a) {
		return assign(a);
	}

	explicit operator bool() {
		get_run_state()->commit_uncommitted();
		return get_run_state()->get_next_bool(block_expr);
	}

	template <typename... arg_types>
	builder operator()(const arg_types &...args) {
		if (builder_precheck()) {
			return create_builder_from_sequence();
		}

		get_run_state()->remove_node_from_sequence(block_expr);
		tracer::tag offset = tracer::get_offset_in_function();

		block::function_call_expr::Ptr expr = std::make_shared<block::function_call_expr>();
		expr->static_offset = offset;

		expr->expr1 = block_expr;
		expr->args = extract_call_arguments<builder>(args...);
		std::reverse(expr->args.begin(), expr->args.end());
		get_run_state()->add_node_to_sequence(expr);

		builder ret_builder;
		ret_builder.block_expr = expr;
		push_to_sequence(expr);
		return ret_builder;
	}

};

void annotate(std::string);

void create_return_stmt(const builder &a);

// Helper function for the implementation of () operator on builder
template <typename BT>
std::vector<block::expr::Ptr> extract_call_arguments_helper(void) {
	std::vector<block::expr::Ptr> empty_vector;
	return empty_vector;
}

template <typename BT, typename T, typename... arg_types>
typename std::enable_if<std::is_convertible<T, BT>::value && !std::is_same<BT, T>::value,
			std::vector<block::expr::Ptr>>::type
extract_call_arguments_helper(const T &first_arg, const arg_types &...rest_args) {
	return extract_call_arguments_helper<BT>((BT)first_arg, rest_args...);
}

template <typename BT, typename... arg_types>
std::vector<block::expr::Ptr> extract_call_arguments_helper(const BT &first_arg, const arg_types &...rest_args) {
	get_run_state()->remove_node_from_sequence(first_arg.block_expr);

	std::vector<block::expr::Ptr> rest = extract_call_arguments_helper<BT>(rest_args...);
	rest.push_back(first_arg.block_expr);
	return rest;
}

template <typename BT, typename... arg_types>
std::vector<block::expr::Ptr> extract_call_arguments(const arg_types &...args) {
	return extract_call_arguments_helper<BT>(args...);
}

} // namespace builder
#endif

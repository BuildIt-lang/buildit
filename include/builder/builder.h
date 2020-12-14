#ifndef BUILDER_H
#define BUILDER_H

#include "blocks/var.h"
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

class var;

template <typename T>
class static_var;

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
	

// All the costructors and copy constructors to the top

	// Simple constrcutor, should only be used inside the operator
	// and set the block_expr immediately
	builder_base() = default;
	// Copy constructor from another builder
	builder_base(const BT& other) {
		block_expr = other.block_expr;
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

	builder_base(const var &a);


	template <typename T>
	builder_base(const static_var<T> &a) : builder_base<BT>((const T)a) {}


// Other basic functions	
	template <typename T>
	BT builder_unary_op() const {
		assert(builder_context::current_builder_context != nullptr);
		if (builder_context::current_builder_context->bool_vector.size() > 0)
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
		assert(builder_context::current_builder_context != nullptr);
		if (builder_context::current_builder_context->bool_vector.size() > 0)
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
		assert(builder_context::current_builder_context != nullptr);
		if (builder_context::current_builder_context->bool_vector.size() > 0)
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
		assert(builder_context::current_builder_context != nullptr);
		if (builder_context::current_builder_context->bool_vector.size() > 0)
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

	explicit operator bool() {
		builder_context::current_builder_context->commit_uncommitted();
		return get_next_bool_from_context(builder_context::current_builder_context, block_expr);
	}

	template <typename... arg_types>
	BT operator()(const arg_types &... args) {
		assert(builder_context::current_builder_context != nullptr);
		if (builder_context::current_builder_context->bool_vector.size() > 0)
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

template <typename T1, typename T2>
struct allowed_builder_type {
	constexpr static bool value = (std::is_base_of<builder_base<T1>, T1>::value && std::is_base_of<builder_base<T2>, T2>::value) ||
		     (std::is_base_of<builder_base<T1>, T1>::value && std::is_convertible<T2, T1>::value) || 
		     (std::is_base_of<builder_base<T2>, T2>::value && std::is_convertible<T1, T2>::value);

};
template <typename T1, typename T2, class Enable = void>
struct allowed_builder_return {
	typedef T2 type;
};
template <typename T1, typename T2>
struct allowed_builder_return<T1, T2, typename std::enable_if<std::is_base_of<builder_base<T1>, T1>::value>::type> {
	typedef T1 type;
};

#include "builder/operator_overload.h.inc"

template <typename BT> 
BT operator!(const builder_base<BT> &a) {
	return a.template builder_unary_op<block::not_expr>();
}

class builder: public builder_base<builder> {
public:
	using builder_base<builder>::builder_base;
	builder operator=(const builder &a) {
		return assign(a);
	}
	using builder_base<builder>::operator[];
};


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

	// operator builder () const;

	explicit operator bool();


	template <typename... types>
	builder operator()(const types &... args) {
		return ((builder) * this)(args...);
	}

	builder operator=(const var &a) { return (builder) * this = a; }
	builder operator[] (const builder &a) {return ((builder) *this)[a];}
	builder operator=(const builder &a) {return (builder)*this = a;}

	virtual ~var() = default;

	builder operator!();
};

template <typename T1, typename T2>
struct allowed_var_type {
	constexpr static bool value = !(std::is_base_of<builder_base<T1>, T1>::value || std::is_base_of<builder_base<T2>, T1>::value) &&
				 (
				      (std::is_base_of<var, T1>::value && std::is_convertible<T2, builder>::value) ||
				      (std::is_base_of<var, T2>::value && std::is_convertible<T1, builder>:: value)
				 );
};

template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator&&(const T1 &a, const T2 &b) { return (builder)a && (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator||(const T1 &a, const T2 &b) { return (builder)a || (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator+(const T1 &a, const T2 &b) { return (builder)a + (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator-(const T1 &a, const T2 &b) { return (builder)a - (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator*(const T1 &a, const T2 &b) { return (builder)a * (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator/(const T1 &a, const T2 &b) { return (builder)a / (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator<(const T1 &a, const T2 &b) { return (builder)a < (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator>(const T1 &a, const T2 &b) { return (builder)a > (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator<=(const T1 &a, const T2 &b) { return (builder)a <= (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator>=(const T1 &a, const T2 &b) { return (builder)a >= (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator==(const T1 &a, const T2 &b) { return (builder)a == (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator!=(const T1 &a, const T2 &b) { return (builder)a != (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator%(const T1 &a, const T2 &b) { return (builder)a % (builder)b;}

template <typename T>
class type_extractor {
public:
	static block::type::Ptr extract_type(void);
};

template <>
class type_extractor<short int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::SHORT_INT_TYPE;
		return type;
	}
};

template <>
class type_extractor<unsigned short int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::UNSIGNED_SHORT_INT_TYPE;
		return type;
	}
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

template <>
class type_extractor<unsigned int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::UNSIGNED_INT_TYPE;
		return type;
	}
};
template <>
class type_extractor<long int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::LONG_INT_TYPE;
		return type;
	}
};

template <>
class type_extractor<unsigned long int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::UNSIGNED_LONG_INT_TYPE;
		return type;
	}
};
template <>
class type_extractor<long long int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::LONG_LONG_INT_TYPE;
		return type;
	}
};

template <>
class type_extractor<unsigned long long int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::UNSIGNED_LONG_LONG_INT_TYPE;
		return type;
	}
};

template <>
class type_extractor<char> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::CHAR_TYPE;
		return type;
	}
};

template <>
class type_extractor<unsigned char> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::UNSIGNED_CHAR_TYPE;
		return type;
	}
};

template <>
class type_extractor<float> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::FLOAT_TYPE;
		return type;
	}
};

template <>
class type_extractor<double> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::DOUBLE_TYPE;
		return type;
	}
};

template <typename T>
class type_extractor<T *> {
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
std::vector<block::type::Ptr> extract_type_vector_dyn<>(void);

template <typename r_type, typename... a_types>
class type_extractor<r_type(a_types...)> {
public:
	static block::type::Ptr extract_type(void) {
		block::function_type::Ptr type = std::make_shared<block::function_type>();
		type->return_type = type_extractor<r_type>::extract_type();
		type->arg_types = extract_type_vector_dyn<a_types...>();
		std::reverse(type->arg_types.begin(), type->arg_types.end());
		return type;
	}
};

// Type extractor for complete closure
template <typename T>
class type_extractor<dyn_var<T>> {
public:
	static block::type::Ptr extract_type(void) {
		block::builder_var_type::Ptr type = std::make_shared<block::builder_var_type>();
		type->builder_var_type_id = block::builder_var_type::DYN_VAR;
		type->closure_type = type_extractor<T>::extract_type();
		return type;
	}
};
template <typename T>
class type_extractor<static_var<T>> {
public:
	static block::type::Ptr extract_type(void) {
		block::builder_var_type::Ptr type = std::make_shared<block::builder_var_type>();
		type->builder_var_type_id = block::builder_var_type::STATIC_VAR;
		type->closure_type = type_extractor<T>::extract_type();
		return type;
	}
};

template <typename T>
class dyn_var : public var {
public:
	using var::operator=;
	// using var::operator builder;

	builder operator=(const dyn_var<T> &a) { return (builder) * this = a; }
	
	static block::type::Ptr create_block_type(void) { return type_extractor<T>::extract_type(); }
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
		tracer::tag offset = get_offset_in_function();
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
	dyn_var(bool create_without_context = false) { create_dyn_var(create_without_context); }
	dyn_var(const dyn_var<T> &a) : dyn_var<T>((builder)a) {}
	template <typename TO>
	dyn_var(const dyn_var<TO> &a) : dyn_var<TO>((builder)a) {}
	template <typename TO>
	dyn_var(const static_var<TO> &a) : dyn_var<T>((TO)a) {}
	dyn_var(const builder a) {
		builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);
		create_dyn_var();
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block_decl_stmt->init_expr = a.block_expr;
	}
	dyn_var(const int &a) : dyn_var((builder)a) {}
	dyn_var(const double &a) : dyn_var((builder)a) {}
	dyn_var(const float &a) : dyn_var((builder)a) {}
	dyn_var(const std::initializer_list<builder> &_a) {
		std::vector<builder> a(_a);

		assert(builder_context::current_builder_context != nullptr);
		for (unsigned int i = 0; i < a.size(); i++) {
			builder_context::current_builder_context->remove_node_from_sequence(a[i].block_expr);
		}
		create_dyn_var();
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;

		tracer::tag offset = get_offset_in_function();
		block::initializer_list_expr::Ptr list_expr = std::make_shared<block::initializer_list_expr>();
		list_expr->static_offset = offset;
		for (unsigned int i = 0; i < a.size(); i++) {
			list_expr->elems.push_back(a[i].block_expr);
		}
		block_decl_stmt->init_expr = list_expr;
	}
	virtual ~dyn_var() = default;

	template <typename TO>
	builder operator=(const dyn_var<TO> &a) {
		return (builder) * this = a;
	}
	builder operator=(const int &a) { return operator=((builder)a); }
	builder operator=(const double &a) { return operator=((builder)a); }

	template <typename Ts>
	builder operator=(const static_var<Ts> &a) {
		return operator=((builder)a);
	}
};


void annotate(std::string);

// The implementation for () operator on builder


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
		return builder::sentinel_builder;
	BT ret_builder;
	ret_builder.block_expr = create_foreign_expr(t);
	return ret_builder;
}

void create_return_stmt(const builder a);


template<typename BT>
builder_base<BT>::builder_base(const var &a) {
	assert(builder_context::current_builder_context != nullptr);
	block_expr = nullptr;
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return;
	assert(a.block_var != nullptr);
	tracer::tag offset = get_offset_in_function();

	block::var_expr::Ptr var_expr = std::make_shared<block::var_expr>();
	var_expr->static_offset = offset;

	var_expr->var1 = a.block_var;
	builder_context::current_builder_context->add_node_to_sequence(
	    var_expr);

	block_expr = var_expr;
}



} // namespace builder
#endif

#ifndef BUILDER_OPERATOR_OVERLOAD_H
#define BUILDER_OPERATOR_OVERLOAD_H

#include "builder/builder_base.h"

namespace builder {

// Helper type to restrict the overloads for specific types
template <typename T1, typename T2>
struct allowed_builder_type {
	constexpr static bool value = (std::is_base_of<builder_base<T1>, T1>::value && std::is_base_of<builder_base<T2>, T2>::value) ||
		     (std::is_base_of<builder_base<T1>, T1>::value && std::is_convertible<T2, T1>::value) || 
		     (std::is_base_of<builder_base<T2>, T2>::value && std::is_convertible<T1, T2>::value);

};

// Helper type to identify the return type for the overloads
template <typename T1, typename T2, class Enable = void>
struct allowed_builder_return {
	typedef T2 type;
};
template <typename T1, typename T2>
struct allowed_builder_return<T1, T2, typename std::enable_if<std::is_base_of<builder_base<T1>, T1>::value>::type> {
	typedef T1 type;
};


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator && (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::and_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator || (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::or_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator + (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::plus_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator - (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::minus_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator * (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::mul_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator / (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::div_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator < (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::lt_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator > (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::gt_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator <= (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::lte_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator >= (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::gte_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator == (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::equals_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator != (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::ne_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator % (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::mod_expr>((ret_type)b);
}

// Unary operators
template <typename BT> 
BT operator!(const builder_base<BT> &a) {
	return a.template builder_unary_op<block::not_expr>();
}

template <typename BT>
void create_return_stmt(const builder_base<BT> &a) {
	assert(builder_context::current_builder_context != nullptr);
	builder_context::current_builder_context->remove_node_from_sequence(
	    a.block_expr);
	assert(builder_context::current_builder_context->current_block_stmt !=
	       nullptr);
	builder_context::current_builder_context->commit_uncommitted();

	block::return_stmt::Ptr ret_stmt =
	    std::make_shared<block::return_stmt>();
	ret_stmt->static_offset = a.block_expr->static_offset;
	ret_stmt->return_val = a.block_expr;
	builder_context::current_builder_context->add_stmt_to_current_block(
	    ret_stmt);
}


}

#endif

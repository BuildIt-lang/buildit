#ifndef BUILDER_OPERATOR_OVERLOAD_H
#define BUILDER_OPERATOR_OVERLOAD_H

#include "builder/builder_base.h"

namespace builder {

template <typename T>
struct is_possible_builder {
	constexpr static bool value = is_builder_type<T>::value || is_dyn_var_type<T>::value;
};


template <typename T, class Enable = void>
struct return_type_helper {
};

template <typename T>
struct return_type_helper <T, typename std::enable_if<is_builder_type<T>::value>::type> {
	typedef T type;
};
template <typename T>
struct return_type_helper <T, typename std::enable_if<!is_builder_type<T>::value && is_dyn_var_type<T>::value>::type> {
	typedef typename T::associated_BT type;
};

template <typename T1, typename T2, class Enable = void>
struct allowed_builder_return {
};

template <typename T1, typename T2>
struct allowed_builder_return<T1, T2, typename std::enable_if<is_possible_builder<T1>::value && std::is_convertible<T2, typename return_type_helper<T1>::type>::value>::type>: return_type_helper<T1> {
};

template <typename T1, typename T2>
struct allowed_builder_return<T1, T2, typename std::enable_if<!is_possible_builder<T1>::value && is_possible_builder<T2>::value && std::is_convertible<T1, typename return_type_helper<T2>::type>::value>::type>: return_type_helper<T2> {
};

template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator && (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::and_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator || (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::or_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator + (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::plus_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator - (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::minus_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator * (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::mul_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator / (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::div_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator < (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::lt_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator > (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::gt_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator <= (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::lte_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator >= (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::gte_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator == (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::equals_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator != (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::ne_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename allowed_builder_return<T1, T2>::type operator % (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::mod_expr>((ret_type)b);
}

// Unary operators
template <typename T> 
typename return_type_helper<T>::type operator!(const T &a) {
	typedef typename return_type_helper<T>::type ret_type;
	return ret_type(a).template builder_unary_op<block::not_expr>();
}

template <typename T>
typename return_type_helper<T>::type operator&(const T &a) {
	typedef typename return_type_helper<T>::type ret_type;
	return ret_type(a).template builder_unary_op<block::addr_of_expr>();
}



}

#endif

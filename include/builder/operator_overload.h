#ifndef BUILDER_OPERATOR_OVERLOAD_H
#define BUILDER_OPERATOR_OVERLOAD_H
#include "builder/to_expr.h"
#include <utility>
namespace builder {

// We use T& and not T in declval so that operators
// that expect a lvalue like & also work
template <typename T1, typename V=void>
struct stripped {
	static auto val(void) -> decltype(std::declval<T1&>());
};

template <typename T1>
struct stripped<dyn_var<T1>> {
	static auto val(void) -> decltype(std::declval<T1&>());
};

template <typename T1, typename T2>
struct allowed_types{};

template <typename T1, typename T2>
struct allowed_types<dyn_var<T1>, dyn_var<T2>> {
	using type = void;
};
template <typename T1, typename T2>
struct allowed_types<dyn_var<T1>, T2> {
	using type = void;
};
template <typename T1, typename T2>
struct allowed_types<T1, dyn_var<T2>> {
	using type = void;
};


template <typename T, typename T1, typename T2>
block::expr::Ptr binary_expr_helper(const T1& d1, const T2& d2) {
	auto e1 = to_expr(d1);
	auto e2 = to_expr(d2);
	auto e = create_expr<T>({e1, e2});
	e->expr1 = e1;
	e->expr2 = e2;
	return e;	
}
template <typename T, typename T1>
block::expr::Ptr unary_expr_helper(const T1& d1) {
	auto e1 = to_expr(d1);
	auto e = create_expr<T>({e1});
	e->expr1 = e1;
	return e;	
}

#define BINARY_OPERATOR(op, op_class) \
template <typename T1, typename T2, typename V = typename allowed_types<T1, T2>::type,  \
	typename RetType = dyn_var<decltype(stripped<T1>::val() op stripped<T2>::val())>> \
auto operator op (const T1& d1, const T2& d2) -> RetType& { \
	return *get_invocation_state()->get_arena()->allocate<RetType>(binary_expr_helper<block::op_class>(d1, d2)); \
} 

// This uses declval and not stripped, because we don't want to strip too 
// many layers if nested
#define UNARY_OPERATOR(op, op_class) \
template <typename T1, typename RetType = dyn_var<decltype(op std::declval<T1&>())>> \
auto operator op (const dyn_var<T1>& d1) -> RetType& { \
	return *get_invocation_state()->get_arena()->allocate<RetType>(unary_expr_helper<block::op_class>(d1)); \
} 

// Binary operators
// Arithmetic Operators
BINARY_OPERATOR(+, plus_expr)
BINARY_OPERATOR(-, minus_expr)
BINARY_OPERATOR(*, mul_expr)
BINARY_OPERATOR(/, div_expr)
BINARY_OPERATOR(%, mod_expr)
// Relational Operators
BINARY_OPERATOR(==, equals_expr)
BINARY_OPERATOR(!=, ne_expr)
BINARY_OPERATOR(<, lt_expr)
BINARY_OPERATOR(>, gt_expr)
BINARY_OPERATOR(<=, lte_expr)
BINARY_OPERATOR(>=, gte_expr)
// Bitwise Operators
BINARY_OPERATOR(&, bitwise_and_expr)
BINARY_OPERATOR(|, bitwise_or_expr)
BINARY_OPERATOR(^, bitwise_xor_expr)
BINARY_OPERATOR(<<, lshift_expr)
BINARY_OPERATOR(>>, rshift_expr)
// Logical Operators
BINARY_OPERATOR(&&, and_expr)
BINARY_OPERATOR(||, or_expr)

// Unary Operators
// Arithmetic Operators
UNARY_OPERATOR(-, unary_minus_expr)
// Bitwise Operators
UNARY_OPERATOR(~, bitwise_not_expr)
// Logical Operators
UNARY_OPERATOR(!, not_expr)
// Other Operators
UNARY_OPERATOR(&, addr_of_expr)

#undef BINARY_OPERATOR
#undef UNARY_OPERATOR
// Helpers
// Prefix increment
template <typename T>
dyn_var<T>& operator ++ (dyn_var<T>& d1) {
	return (d1 = d1 + 1);
}
// Postfix increment
template <typename T>
dyn_var<T>& operator ++ (dyn_var<T>& d1, int) {
	return ((d1 = d1 + 1) - 1);
}
// Prefix decrement
template <typename T>
dyn_var<T>& operator -- (dyn_var<T>& d1) {
	return (d1 = d1 - 1);
}
// Postfix increment
template <typename T>
dyn_var<T>& operator -- (dyn_var<T>& d1, int) {
	return ((d1 = d1 - 1) + 1);
}

// Hybrid Assignment 
template <typename T1, typename T2>
dyn_var<T1>& operator += (dyn_var<T1>& d1, const T2& d2) {
	return (d1 = d1 + d2);
}
template <typename T1, typename T2>
dyn_var<T1>& operator -= (dyn_var<T1>& d1, const T2& d2) {
	return (d1 = d1 - d2);
}
template <typename T1, typename T2>
dyn_var<T1>& operator *= (dyn_var<T1>& d1, const T2& d2) {
	return (d1 = d1 * d2);
}
template <typename T1, typename T2>
dyn_var<T1>& operator /= (dyn_var<T1>& d1, const T2& d2) {
	return (d1 = d1 / d2);
}
template <typename T1, typename T2>
dyn_var<T1>& operator &= (dyn_var<T1>& d1, const T2& d2) {
	return (d1 = d1 & d2);
}
template <typename T1, typename T2>
dyn_var<T1>& operator |= (dyn_var<T1>& d1, const T2& d2) {
	return (d1 = d1 | d2);
}
template <typename T1, typename T2>
dyn_var<T1>& operator ^= (dyn_var<T1>& d1, const T2& d2) {
	return (d1 = d1 ^ d2);
}

// An explicit cast expression, this can however be automatically inserted
// by comparing types while assignment, but for now, we will keep this simple
template <typename T, typename T2>
dyn_var<T>& cast_to(const T2& d1) {
	auto ce = block::to<block::cast_expr>(unary_expr_helper<block::cast_expr>(d1));
	ce->type1 = type_extractor<T>::extract_type();
	return *get_invocation_state()->get_arena()->allocate<dyn_var<T>>(ce); 
}

}
#endif

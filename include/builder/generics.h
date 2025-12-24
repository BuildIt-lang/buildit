#ifndef BUILDER_GENERICS_H
#define BUILDER_GENERICS_H

#include "blocks/var.h"
#include "builder/dyn_var.h"

namespace builder {

// TODO: Figure out if this needs to be wrapped into a generics sub namespace

// Generic placeholder class to be passed inside dyn_var
class generic {
public:
	// Define the dereference_type to help the provider
	using dereference_type = generic;
};

// For the generic type, declare all the operators so the types can be inferred for the wrapper
// We just have to declare but not define since the operators are only invoked in unevalauted contexts

#define BINARY_OPERATOR(op) \
template <typename T> \
generic operator op (const generic&, const T&); \
template <typename T> \
generic operator op (const T&, generic&); \
generic operator op (const generic&, const generic&); 

#define UNARY_OPERATOR(op) generic operator op (const generic&);
// Binary operators
// Arithmetic Operators
BINARY_OPERATOR(+)
BINARY_OPERATOR(-)
BINARY_OPERATOR(*)
BINARY_OPERATOR(/)
BINARY_OPERATOR(%)
// Relational Operators
BINARY_OPERATOR(==)
BINARY_OPERATOR(!=)
BINARY_OPERATOR(<)
BINARY_OPERATOR(>)
BINARY_OPERATOR(<=)
BINARY_OPERATOR(>=)
// Bitwise Operators
BINARY_OPERATOR(&)
BINARY_OPERATOR(|)
BINARY_OPERATOR(^)
BINARY_OPERATOR(<<)
BINARY_OPERATOR(>>)
// Logical Operators
BINARY_OPERATOR(&&)
BINARY_OPERATOR(||)

// Unary Operators
// Arithmetic Operators
UNARY_OPERATOR(-)
// Bitwise Operators
UNARY_OPERATOR(~)
// Logical Operators
UNARY_OPERATOR(!)
// Other Operators
UNARY_OPERATOR(&)

#undef BINARY_OPERATOR
#undef UNARY_OPERATOR

// An opaque handle over block::type
// this also allows us to overload operators and helper functions
class type {
public:
	block::type::Ptr enclosed_type;
	type(block::type::Ptr t): enclosed_type(t) {}

	bool operator==(const type& other) {
		return enclosed_type->is_same(other.enclosed_type);
	}

	bool operator!=(const type& other) {
		return !(*this == other);
	}
};

type type_of(const dyn_var_base& v);

// With type constructor helper for dyn_var
struct with_type {
	type t;
	block::expr::Ptr init_expr;
	with_type(const type& t): t(t), init_expr(nullptr) {}
	template <typename T>
	with_type(const type& t, const T &a): t(t), init_expr(to_expr(a)) {}
	with_type(const dyn_var_base& v): t(type_of(v)), init_expr(to_expr(v)) {}
};

template <typename T>
dyn_var_impl<T>::dyn_var_impl(const with_type& wt) {
	create_standalone(wt.init_expr);
	// Currently we are supporting generic only 
	// for standalone vars
	assert(var_mode == standalone_var);
	block_var->var_type = wt.t.enclosed_type;
}

template <typename T>
type create_type(void) {
	return type(type_extractor<T>::extract_type());
}


type array_of(const type &t, int size = -1);
type remove_array(const type &t);
type remove_array(const type &t, int& size);
bool is_array(const type &t);

type pointer_of(const type &t);
bool is_pointer(const type &t);
type remove_pointer(const type &t);

}

#endif

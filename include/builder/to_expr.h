#ifndef BUILDER_TO_EXPR_H
#define BUILDER_TO_EXPR_H
#include <utility>
#include "builder/forward_declarations.h"
namespace builder {

// A single templated constructor to create expr blocks of any type
// This handles removing childred from UC, adding new expresisons to UC 
// and caching for runs, the children, aren't set but are removed from UC
template <typename T>
typename T::Ptr create_expr(const std::vector<block::expr::Ptr> &children) {
	// Caching happens here
	if (get_run_state()->is_catching_up()) {
		return block::to<T>(get_run_state()->get_next_cached_expr());
	}	
	for (auto c: children) {
		get_run_state()->remove_node_from_sequence(c);
	}		
	tracer::tag offset = tracer::get_offset_in_function();	
	typename T::Ptr expr = std::make_shared<T>();
	expr->static_offset = offset;
	// The other members will be set by the caller
	get_run_state()->add_node_to_sequence(expr);	
	// For caching
	get_run_state()->add_to_cached_expr(expr);
	return expr;
}


// Single conversion function to convert anything into 
// an expression, overloads for different types
block::expr::Ptr to_expr(const dyn_var_base& d);

template <typename T>
typename std::enable_if<std::is_integral<T>::value, block::expr::Ptr>::type to_expr(const T& v) {
	auto e = create_expr<block::int_const>({});
	e->value = v;
	e->is_64bit = sizeof(T) > 4;
	return e;
}

template <typename T>
block::expr::Ptr to_expr(const static_var<T>& s) {
	return to_expr((T)s);
}

template <typename T>
block::expr::Ptr to_expr(const std::vector<T>& v) {
	std::vector<block::expr::Ptr> argv;
	for (auto x: v) {
		argv.push_back(to_expr(x));
	}	
	auto e = create_expr<block::initializer_list_expr>(argv);
	e->elems = argv;
	return e;
	
}

block::expr::Ptr to_expr(const float& v);
block::expr::Ptr to_expr(const double& v);
block::expr::Ptr to_expr(const std::string& s);
block::expr::Ptr to_expr(const char* s);
block::expr::Ptr to_expr(char* s);

// An untyped expression wrapper
// to be specifically used for initializer_lists since they need 
// to be homogenous. This can be constructed from anything that is
// to_expr convertible

struct expr_wrapper_base {
	block::expr::Ptr e;
	expr_wrapper_base(block::expr::Ptr e): e(e) {}
};

struct expr_wrapper: public expr_wrapper_base {
	template<typename T>
	expr_wrapper(const T& t): expr_wrapper_base(to_expr(t)) {}
};

block::expr::Ptr to_expr(const std::initializer_list<expr_wrapper>& i);
block::expr::Ptr to_expr(const expr_wrapper_base&);


}

#endif

#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include <iostream>

// We will now created our own builder types and dyn_var types using CRTP

template <typename T>
class my_dyn_var;
class my_builder;


template <int recur>
struct member_accessible: public builder::member_base_impl<my_builder> {
	using member_base_impl::member_base_impl;	
	// Define all the members here
	member_accessible <recur-1> var1 = member_accessible<recur-1>(this, "var1");
	member_accessible <recur-1> neighbor = member_accessible<recur-1>(this, "neighbor");
};

template <>
struct member_accessible<0>: public builder::member_base {
	member_accessible(builder::member_base* p, std::string mn) {}
};




class my_builder: public builder::builder_base<my_builder>, public member_accessible<3> {
public:
	using builder::builder_base<my_builder>::builder_base;
	my_builder operator=(const my_builder &a) {
		return assign(a);
	}
	using builder::builder_base<my_builder>::operator[];
	
	virtual block::expr::Ptr get_parent() const {
		return block_expr;
	}
	
	// This is required so that when my_builders are copied the members don't retain a reference to the 
	// old object
	my_builder(const my_builder& a) : builder_base<my_builder>(a), member_accessible<3>() {
		block_expr = a.block_expr;
	}
	
};

template <typename T>
class my_dyn_var : public builder::dyn_var_base<T, my_dyn_var<T>, my_builder>, public member_accessible<3> {
public:
	virtual ~my_dyn_var() = default;
	using builder::dyn_var_base<T, my_dyn_var<T>, my_builder>::dyn_var_base;
	using builder::dyn_var_base<T, my_dyn_var<T>, my_builder>::operator[];
	using builder::dyn_var_base<T, my_dyn_var<T>, my_builder>::operator=;

	my_builder operator=(const my_dyn_var<T> &a) {
		return (*this = (my_builder)a);
	}

	virtual block::expr::Ptr get_parent() const {
		return ((my_builder) (*this)).get_parent();
	}

};




int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(
	    [=](my_dyn_var<int> x) -> my_dyn_var<int> {
		    my_dyn_var<int> z = (3 + x).var1 + (5 * x).var1;
		    return z.neighbor + 2;
	    },
	    "func1");
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

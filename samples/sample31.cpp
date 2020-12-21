#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include <iostream>

// We will now created our own builder types and dyn_var types using CRTP and Mixin

template <int recur>
struct member_accessible;

using my_builder = builder::builder_final<member_accessible<3>>;
template <typename T>
using my_dyn_var = builder::dyn_var_final<T, member_accessible<3>, my_builder>;

template <int recur>
struct member_accessible: public builder::member_base_impl<my_builder> {
	using member_base_impl::member_base_impl;	
	// Define all the members here
	member_accessible <recur-1> var1 = member_accessible<recur-1>(this, "var1");
	member_accessible <recur-1> neighbor = member_accessible<recur-1>(this, "neighbor");
};

template <>
struct member_accessible<0>: public builder::member_base {
	using member_base::member_base;
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

#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

using builder::as_member;
using builder::dyn_var;
// We will now create our own dyn_var type with members
// We are deprecating extending dyn_var with members
// instead one can write a specialization for a particular type

struct foo {
	static constexpr const char* type_name = "foo";
};

template <typename T>
struct my_dyn_var: public builder::dyn_var<T> {
	using builder::dyn_var<T>::dyn_var;
	using builder::dyn_var<T>::operator=;

	my_dyn_var(const my_dyn_var &t) : builder::dyn_var<foo>((builder::builder)t) {}

	builder::builder operator=(const my_dyn_var<T> &t) {
		return (*this) = (builder::builder)t;
	}
	dyn_var<int> var1 = as_member(this, "var1");
	dyn_var<int> neighbor = as_member(this, "neighbor");
};

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(
	    [=](my_dyn_var<foo> x) -> dyn_var<int> {
		    my_dyn_var<foo> z = 3 + x.var1 + 5 * x.var1;
		    return z.neighbor + 2;
	    },
	    "func1");
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

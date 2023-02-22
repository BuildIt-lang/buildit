#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

using builder::as_member;
using builder::dyn_var;
// We will now create our own dyn_var type with members
template <typename T>
struct my_dyn_var : public dyn_var<T> {
	using dyn_var<T>::dyn_var;
	using dyn_var<T>::operator=;

	my_dyn_var(const my_dyn_var &t) : dyn_var<T>((builder::builder)t) {}
	template <typename TO>
	my_dyn_var(const my_dyn_var<TO> &t) : dyn_var<T>((builder::builder)t) {}
	template <typename TO>
	builder::builder operator=(const my_dyn_var<TO> &t) {
		return (*this) = (builder::builder)t;
	}
	builder::builder operator=(const my_dyn_var &t) {
		return (*this) = (builder::builder)t;
	}
	dyn_var<int> var1 = as_member(this, "var1");
	dyn_var<int> neighbor = as_member(this, "neighbor");
};

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(
	    [=](my_dyn_var<int> x) -> my_dyn_var<int> {
		    my_dyn_var<int> z = 3 + x.var1 + 5 * x.var1;
		    return z.neighbor + 2;
	    },
	    "func1");
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

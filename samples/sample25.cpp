/*NO_TEST*/
#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;

static dyn_var<int> foo(dyn_var<int> x) {
	return x + 1;
}

static void bar(dyn_var<int> x) {
	dyn_var<int> b = x;
	x = b + 1;
}

static dyn_var<int> foobar(dyn_var<int> x, int &iter, dyn_var<void(int)> &print_val) {
	dyn_var<int> t = 0;
	for (static_var<int> i = 0; i < iter; i++)
		t = t + x + 1;
	print_val(t);
	return t;
}
int main(int argc, char *argv[]) {
	builder::builder_context context;

	auto ast = context.extract_function_ast(
	    [=](dyn_var<int> x, dyn_var<int> y, dyn_var<int> w) -> dyn_var<int> {
		    dyn_var<int> z = x - y - w;
		    return 0;
	    },
	    "func1");
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);

	auto ast2 = context.extract_function_ast(foo, "func2");
	ast2->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast2, std::cout, 0);

	auto ast3 = context.extract_function_ast(bar, "func3");
	ast3->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast3, std::cout, 0);

	auto &print_val = *(context.assume_variable<dyn_var<void(int)>>("print_val"));
	auto ast4 = context.extract_function_ast(foobar, "func4", 2, print_val);
	ast4->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast4, std::cout, 0);
	return 0;
}

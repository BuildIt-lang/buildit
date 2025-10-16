#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include <iostream>
using builder::dyn_var;

// A simple example with an assumed variable in the builder context
// Assumed variables are now deprecated and one can simply use a global
dyn_var<int> assumed_variable = builder::with_name("global_var1");
static void foo(void) {
	dyn_var<int> a;
	a = 20;
	assumed_variable = 10 + a;
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include <iostream>
using builder::dyn_var;

// A simple straight line code that calls another function. Should not detect
// this as a loop
static void bar(void) {
	dyn_var<int> a = 0;
	dyn_var<int> b = a;
	a &&b;
}
static void foo(void) {
	bar();
	bar();
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

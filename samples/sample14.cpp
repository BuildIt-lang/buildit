#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include <iostream>
using builder::dyn_var;

// An example of dyn_var<int> being passed around and returning expressions back
// from the functions
static dyn_var<int> bar(dyn_var<int> x) {
	return x + 1;
}
static void foo(void) {
	dyn_var<int> a = 0;
	dyn_var<int> b = bar(a);
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include <iostream>
using builder::dyn_var;

// A nested loop
static void foo(void) {
	dyn_var<int> a = 0;
	for (dyn_var<int> c = 0; c < 100; c = c + 3) {
		for (dyn_var<int> b = 0; b < 10; b = b + 1) {
			a = a + b;
		}
	}
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

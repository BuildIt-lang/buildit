#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

using builder::dyn_var;
using builder::static_var;

// Static loop combined with dynamic loop
// Outer loop should be unrolled and inner should be a loop
static void foo(void) {

	dyn_var<int> a = 0;
	for (static_var<int> x = 1; x <= 2; x++) {
		for (dyn_var<int> y = 0; y < x * 100; y = y + 1) {
			a = a * 1;
		}
	}
	a = a - 1;
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

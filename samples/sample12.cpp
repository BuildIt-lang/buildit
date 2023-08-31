#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

using builder::dyn_var;
using builder::static_var;

// A simple straight line code enclosed in a static loop. This should be
// unrolled 10 times. There shouldn't be a loop in the AST
static void foo(void) {
	static_var<long> y = 0;
	for (static_var<int> x = y; x < 10; x++) {
		dyn_var<int> a;
		a = a + 1;
	}
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

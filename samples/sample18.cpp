#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

using builder::dyn_var;
using builder::static_var;

// Function with unrolled branches
static void foo(void) {
	dyn_var<int> a;
	for (static_var<int> i = 0; i < 128; i++) {
		if (a) {
			a = a + i;
		} else {
			a = a - i;
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

#include "blocks/c_code_generator.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;

static void bar(void) {
	// This example tests the case where a jump cuts
	// through the parent loop
	dyn_var<int> x = 5;
	while (x < 10) {
		while (x > 100) {
			if (x == 0)
				break;
			x = x + 4;
		}
		x = x + 5;
		while (x != -1)
			x = x + 6;
	}
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

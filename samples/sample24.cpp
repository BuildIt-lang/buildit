#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;

static void bar(void) {
	for (static_var<int> i = 0; i < 10; i++) {
		for (dyn_var<static_var<int>> g = 0; g < i; g = g + 1) {
			dyn_var<dyn_var<int>> x = i;
			x = x + 1;
		}
	}
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	// Code to generate full type closure with builder_var_type
	auto ast = context.extract_ast_from_function(bar);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

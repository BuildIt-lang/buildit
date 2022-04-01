#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include <iostream>
using builder::dyn_var;

// A simple straight line code with array types
static void foo(void) {
	dyn_var<int[10]> array1;
	array1[0] = 1;

	dyn_var<int[]> array2;
	array2[0] = 5;

	dyn_var<int[512][256]> array3;
	array3[0][0] = 1;
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

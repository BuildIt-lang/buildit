#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"
using builder::dyn_var;

// A simple for loop
void foo(void) {
	dyn_var<int> a = 0;
	for (dyn_var<int> b = 0; b < 10; b = b + 1) {
		a = a + b;
	}
}
int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);	
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}


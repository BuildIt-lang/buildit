#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"

using builder::dyn_var;
// A simple function_var declaration
void foo(void) {
	dyn_var<void (int, int*)> bar;
	dyn_var<int ()> bar2;
	
	dyn_var<int> a;
	dyn_var<int*> b;
	bar(a, b);
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);	
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}

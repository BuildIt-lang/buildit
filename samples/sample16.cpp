#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"
using builder::dyn_var;

// A simple example with an assumed variable in the builder context
dyn_var<int> *assumed_variable_ref;
void foo(void) {
	dyn_var<int> a;
	a = 20;
	*assumed_variable_ref = 10 + a;
}
int main(int argc, char* argv[]) {
	builder::builder_context context;
	assumed_variable_ref = context.assume_variable<dyn_var<int>>("global_var1");
	auto ast = context.extract_ast_from_function(foo);	
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}


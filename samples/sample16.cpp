#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"
using int_var = builder::int_var;



// A simple example with an assumed variable in the builder context
int_var *assumed_variable_ref;
void foo(void) {
	int_var a;
	a = 20;
	*assumed_variable_ref = 10 + a;
}
int main(int argc, char* argv[]) {
	builder::builder_context context;
	assumed_variable_ref = context.assume_variable<int_var>("global_var1");
	auto ast = context.extract_ast_from_function(foo);	
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}


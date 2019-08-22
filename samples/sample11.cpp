#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"
using int_var = builder::int_var;



// A simple straight line code that calls another function. Should not detect this as a loop
void bar(void) {
	int_var a = 0;
	int_var b = a;
	a && b;
}
void foo(void) {
	bar();
	bar();
}
int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);	
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}


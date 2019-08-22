#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"
using int_var = builder::int_var;



// An example of int_var being passed around and returning expressions back from the functions
int_var bar(int_var x) {
	return x + 1;	
}
void foo(void) {
	int_var a = 0;
	int_var b = bar(a);
}
int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);	
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}

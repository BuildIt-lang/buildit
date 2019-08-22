#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"
using int_var = builder::int_var;



// A simple for loop with break and continue
void foo(void) {
	int_var a = 0;
	for (int_var b = 0; b < 10; b = b + 1) {
		if (b == 5)
			continue;
		a = a + b;
		if (a > 25)
			break;
	}
}
int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);	
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}


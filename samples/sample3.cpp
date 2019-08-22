#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"
using int_var = builder::int_var;

// A nested if condition 
void foo(void) {
	int_var a;
	int_var b;
	int_var c;

	if (!c) {
		a &&b;
		if (a && b) {
			c && b;
		} else {
			b && c;
		}
	} else {
		b && a;
		if (b && a) {
			b && c;
		} else {
			c && b;
		}
	}
	// This statement should appear AFTER the if statement and not duplicated
	// in both the above branches
	c && b;
	
}
int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);	
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}


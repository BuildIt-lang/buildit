#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include <iostream>
using builder::dyn_var;

// A simple if condition with two stmts
static void foo(void) {
	dyn_var<int> a;
	dyn_var<int> b;
	dyn_var<int> c;

	if (c < 10) {
		a &&b;
	} else {
		b &&a;
	}
	// This statement should appear AFTER the if statement and not
	// duplicated in both the above branches
	c &&b;

	a = (c < 10) ? c : b;
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

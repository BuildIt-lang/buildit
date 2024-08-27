#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include <iostream>
using builder::dyn_var;

// A simple straight line code with 2 variable declarations and one operator
static void foo(void) {
	dyn_var<int> a = 0;
	dyn_var<int> b = a;
	a &&b;
	a &b;
	b + 1;
	1 + b;
	a % 2;
	a | 2;
	a >> 2;
	b << 3;
	a ^ b;
	a+=b;
	b*=b;
	a&=b;
	a|=b;
	a^=b;
	~b;
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

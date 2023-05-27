#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;

static dyn_var<int> foo(dyn_var<int> x) {
	if (x > 10)
		return 9;
	else
		return 0;
}

static void bar(void) {
	dyn_var<int> x;

	dyn_var<int> t = foo(x);
	if (t) {
		dyn_var<int> k = 0;
		k = 0;
	}

	// x = x + 1;

	if (x) {
		dyn_var<int> k = 0;
		k = 0;
	}
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	// Code to test assignments between static var and normal vars
	auto ast = context.extract_ast_from_function(bar);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;

int main(int argc, char *argv[]) {
	builder::builder_context context;
	// Code to test assignments between static var and normal vars
	auto ast = context.extract_ast_from_lambda([=](void) {
		static_var<int> foo = 4 + 5;
		dyn_var<int> x = foo + 9;
		dyn_var<int> y = 8 + foo;

		x = y + foo;
	});
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

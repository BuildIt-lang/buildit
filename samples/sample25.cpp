/*NO_TEST*/
#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
using builder::dyn_var;
using builder::static_var;

int main(int argc, char* argv[]) {
	builder::builder_context context;
	// Code to test assignments between static var and normal vars
	auto ast = context.extract_function_ast ([=] (dyn_var<int> x, dyn_var<int> y, dyn_var<int> w) {	
		dyn_var <int> z = x - y - w;
	});
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}


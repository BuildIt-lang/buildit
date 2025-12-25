// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;


static void bar(dyn_var<int*> x, dyn_var<int*> y, dyn_var<int> size) {
	x.add_attribute("restrict");
	y.add_attribute("restrict");
	for (builder::dyn_var<int> i = 0; i < size; i++) {
		x[i] = y[i];
	}
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	auto fd = block::to<block::func_decl>(ast);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}


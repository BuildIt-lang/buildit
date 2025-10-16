// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include "blocks/rce.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static void bar(std::vector<int> x) {
	dyn_var<int[]> y = x;
	for (dyn_var<int> i = 0; i < x.size(); ++i) {
		y[i] = y[i] + y[i-1];
	}
} 

int main(int argc, char* argv[]) {
	builder::builder_context context;
	std::vector<int> x = {1, 2, 3};
	auto ast = context.extract_function_ast(bar, "bar", x);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}



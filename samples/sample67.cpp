// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include "blocks/rce.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static void bar(void) {
	builder::annotate("pragma: omp parallel for");
	for (dyn_var<int> outer = 0; outer < 10; ++outer) {
		dyn_var<int> sum = 0;
		builder::annotate("pragma: unroll");
		for (dyn_var<int> inner = 0; inner < 15; ++inner) {
			sum += inner;
		}
	}
} 

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}



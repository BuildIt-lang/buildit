// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/dyn_var.h"
#include "builder/lib/utils.h"
#include "builder/static_var.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static dyn_var<int> isEven(dyn_var<int> x) {
	static_var<int> xs = builder::up_cast_range(x, 16);
	return (xs % 2) == 0;
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	context.run_rce = true;
	block::c_code_generator::generate_code(context.extract_function_ast(isEven, "isEven"), std::cout, 0);
	return 0;
}

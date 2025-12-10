// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

namespace runtime {
static dyn_var<void*(size_t)> malloc = builder::with_name("malloc");
static dyn_var<void(void*)> free = builder::with_name("free");
}

static void bar(void) {
	dyn_var<int*> x = builder::cast_to<int*>(runtime::malloc(sizeof(int)));	
	x[0] = 123;
	runtime::free(x);
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}


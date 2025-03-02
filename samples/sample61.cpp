// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include "blocks/rce.h"
#include "builder/nd_var.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;
using builder::nd_var;


static void bar(void) {
	nd_var<bool> t;
	dyn_var<int> x = (bool)t;
	t.require_equal(true);

	nd_var<bool> r(true);
	dyn_var<int> y = (bool)r;
	r.require_equal(false);
	t.require_equal(true);
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}



// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include "blocks/rce.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static dyn_var<int> power(dyn_var<int> base, dyn_var<int> exp) {
	
	// Make a copy of exponent because we want to make sure it isn't
	// copy eliminated partially

	dyn_var<int> exponent = exp;

	dyn_var<int> res = 1, x = base;
	while (exponent > 1) {
		if (exponent % 2 == 1)
			res = res * x;
		x = x * x;
		exponent = exponent / 2;
	}
	return res * x;	
}

static void bar(void) {

    dyn_var<int> a;
    dyn_var<int> x = a + 1;
    while (x) {
        a = a + 1;
    }

} 

int main(int argc, char* argv[]) {
	
	builder::builder_context context;
	context.run_rce = true;
	auto ast = context.extract_function_ast(power, "power");
	block::c_code_generator::generate_code(ast, std::cout, 0);


	builder::builder_context context2;
	context2.run_rce = true;
	auto ast2 = context2.extract_function_ast(bar, "bar");
	block::c_code_generator::generate_code(ast2, std::cout, 0);

	return 0;
}



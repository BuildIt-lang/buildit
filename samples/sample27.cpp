/*NO_TEST*/
#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;

static dyn_var<int> power_f(dyn_var<int> base, static_var<int> exponent) {
	dyn_var<int> res = 1, x = base;
	while (exponent > 0) {
		if (exponent % 2 == 1)
			res = res * x;
		x = x * x;
		exponent = exponent / 2;
	}
	return res;
}
int main(int argc, char *argv[]) {
	auto ast1 = builder::builder_context().extract_function_ast(
	    power_f, "power_15", 15);
	block::c_code_generator::generate_code(ast1, std::cout, 0);
	return 0;
}

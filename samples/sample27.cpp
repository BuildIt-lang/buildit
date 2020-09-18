/*NO_TEST*/
#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
using builder::dyn_var;
using builder::static_var;

template <typename BT, typename ET>
dyn_var<int> power_f(BT base, ET exponent) {
	dyn_var<int> res = 1, x = base;
	while (exponent > 1) {
		if (exponent % 2 == 1)
			res = res * x;
		x = x * x;
		exponent = exponent / 2;
	}
	return res * x;
}
int main(int argc, char* argv[]) {
	int power = 15;
	auto ast1 = builder::builder_context().extract_function_ast(power_f<dyn_var<int>, static_var<int>>, "power_15", power);
	block::c_code_generator::generate_code(ast1, std::cout, 0);	
	int base =  5;
	auto ast2 = builder::builder_context().extract_function_ast(power_f<static_var<int>, dyn_var<int>>, "power_5", base);	
	block::c_code_generator::generate_code(ast2, std::cout, 0);	

	return 0;
}


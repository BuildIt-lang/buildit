// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/builder_dynamic.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static dyn_var<int> power_f(dyn_var<int> base, static_var<int> exponent) {
	dyn_var<int> res = 1, x = base;
	while (exponent > 1) {
		if (exponent % 2 == 1)
			res = res * x;
		x = x * x;
		exponent = exponent / 2;
	}
	return res * x;
}

int main(int argc, char *argv[]) {
	auto fptr = (int (*)(int))builder::compile_function(power_f, 5);
	std::cout << fptr(8) << std::endl;
	return 0;
}

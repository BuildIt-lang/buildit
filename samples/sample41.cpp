// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/builder_dynamic.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static const char vector_t_name[] = "std::vector";

template <typename T>
using vector_t = builder::name<vector_t_name, T>;

static dyn_var<int> power_f(void) {
	dyn_var<vector_t<int>> x = {0, 1, 2};
	dyn_var<vector_t<int>> y = x;
	for (dyn_var<int> i = 0; i < 5; i = i + 1)
		y[0] = y[1] + y[0];
	return y[0];
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	context.dynamic_use_cxx = true;
	context.dynamic_header_includes = "#include <vector>";
	context.feature_unstructured = true;
	auto fptr = (int (*)(void))builder::compile_function_with_context(context, power_f);
	std::cout << fptr() << std::endl;
	return 0;
}

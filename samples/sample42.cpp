// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/dyn_var.h"
#include "builder/lib/utils.h"
#include "builder/static_var.h"
#include <iostream>
// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static void bar(void) {
	// Insert code to stage here
	dyn_var<int[]> y = 0;
	resize_arr(y, 10);
	for (dyn_var<int> x = 0; x < 10; ++x) {
		*y += x;
		y[x] *= 2;
	}
	for (dyn_var<int> x = 0; x < 10; x++) {
		*y -= x;
		y[x] /= 2;
	}

	(*y)++;
}

int main(int argc, char *argv[]) {
	block::c_code_generator::generate_code(builder::builder_context().extract_function_ast(bar, "bar"), std::cout,
					       0);
	return 0;
}

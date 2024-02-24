/*NO_TEST*/
/* This feature has been disabled for now. TODO: Enable this test after selective path merging implementation */
// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static void bar(void) {
	static_var<int> x = 0;

	dyn_var<int> y = 0;
	dyn_var<int> m, n;

	if (y) {
		x = 1;
	} else {
		x = 2;
	}
	// When z is declared, x is in different states
	dyn_var<int> z = x;
	dyn_var<int &> k = m;

	// Executions can now merge, but z is still in different states
	x = 0;

	// this declaration forces executions to merge because static tags are the same
	// merge is triggered by memoization
	dyn_var<int> b;

	// this statement now has issues because z has forked
	dyn_var<int> a = z;

	z = z + k;
}

int main(int argc, char *argv[]) {
	block::c_code_generator::generate_code(builder::builder_context().extract_function_ast(bar, "bar"), std::cout,
					       0);
	return 0;
}

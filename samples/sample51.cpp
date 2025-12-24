// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;


// Members of this struct will have names except for the first one
// since we currently don't match sizes
struct wrapper1 {
	dyn_var<int> mem1;
	dyn_var<char> mem2;
};

// Members of this struct will have names since the first member is a non dynamic one
struct wrapper2 {
	int spacer;
	dyn_var<int> mem1;
	dyn_var<char> mem2;
};


static void bar(void) {
	struct wrapper1 x;
	struct wrapper2 y;
	x.mem1 = x.mem2;
	y.mem2 = y.mem1;
}

int main(int argc, char *argv[]) {
	block::c_code_generator::generate_code(
		builder::builder_context().extract_function_ast(bar, "bar"), std::cout, 0);
	return 0;
}

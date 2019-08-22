#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
using int_var = builder::int_var;
template <typename T>
using static_var = builder::static_var<T>;



// Static loop combined with dynamic loop
// Outer loop should be unrolled and inner should be a loop
void foo(void) {

	int_var a = 0;
	for (static_var<int> x = 1; x <= 2; x++) {
		for (int_var y = 0; y < x * 100; y = y + 1) {
			a = a * 1;	
		}
	}
	a = a - 1;
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	context.extract_ast_from_function(foo)->dump(std::cout, 0);	
	return 0;
}

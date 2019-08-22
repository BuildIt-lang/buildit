#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
using int_var = builder::int_var;
template <typename T>
using static_var = builder::static_var<T>;



// A simple straight line code enclosed in a static loop. This should be unrolled 10 times. There shouldn't be a loop in the AST
void foo(void) {
	for (static_var<int> x = 0; x < 10; x++) {
		int_var a;
		a = a + 1;
	}	
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	context.extract_ast_from_function(foo)->dump(std::cout, 0);	
	return 0;
}

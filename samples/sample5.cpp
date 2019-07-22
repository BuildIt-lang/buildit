#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
using int_var = builder::int_var;



// A simple for loop
void foo(void) {
	int_var a = 0;
	for(int_var b = 0; !b; b = b + 1) {
		a = a + b;
	}
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	context.extract_ast_from_function(foo)->dump(std::cout, 0);	
	return 0;
}

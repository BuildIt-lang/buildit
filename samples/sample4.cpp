#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
using int_var = builder::int_var;



// A simple straight line code with three variables and assignment
void foo(void) {
	int_var a;
	int_var b;
	int_var c;
	c = a + b;
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	context.extract_ast_from_function(foo)->dump(std::cout, 0);	
	return 0;
}

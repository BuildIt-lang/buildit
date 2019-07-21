#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
using int_var = builder::int_var;



// A simple straight line code with three variables and assignment
void foo(builder::builder_context* context) {
	int_var a(context);
	int_var b(context);
	int_var c(context);
	c = a + b;
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	context.extract_ast_from_function(foo)->dump(std::cout, 0);	
	return 0;
}

#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
using int_var = builder::int_var;

// A simple if condition with two stmts
void foo(builder::builder_context* context) {
	int_var a(context);
	int_var b(context);
	int_var c(context);

	if (c) {
		a &&b;
	} else {
		b && a;
	}
	// This statement should appear AFTER the if statement and not duplicated
	// in both the above branches
	c && b;
	
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	context.extract_ast_from_function(foo)->dump(std::cout, 0);	
	return 0;
}

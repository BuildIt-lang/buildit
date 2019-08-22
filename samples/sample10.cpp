#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
using int_var = builder::int_var;
template <typename T>
using pointer_var = builder::pointer_var<T>;


// Pointer variables
void foo(void) {
	pointer_var<int_var> a;
	a[5] = a[6];
	
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	context.extract_ast_from_function(foo)->dump(std::cout, 0);	
	return 0;
}

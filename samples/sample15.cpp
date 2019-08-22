#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
using int_var = builder::int_var;
template <typename r_type, typename... a_types>
using function_var = builder::function_var<r_type, a_types...>;
using void_var = builder::void_var;

// A simple function_var declaration
void foo(void) {
	function_var<void_var, int_var> bar;
	function_var<int_var> bar2;
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	context.extract_ast_from_function(foo)->dump(std::cout, 0);	
	return 0;
}

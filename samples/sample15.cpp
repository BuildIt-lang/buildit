#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"

using int_var = builder::int_var;
template <typename r_type, typename... a_types>
using function_var = builder::function_var<r_type, a_types...>;
using void_var = builder::void_var;
template <typename T>
using pointer_var = builder::pointer_var<T>;

// A simple function_var declaration
void foo(void) {
	function_var<void_var, int_var, pointer_var<int_var>> bar;
	function_var<int_var> bar2;
		
	int_var a;
	pointer_var<int_var> b;
	bar(a, b);
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);	
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}

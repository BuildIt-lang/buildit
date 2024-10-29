#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include "builder/lib/utils.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;


static void bar(dyn_var<int> x) {
	/* Test out of 	order deinitialization of static variables and arrays */
	
	auto sv1 = new static_var<int>();
	auto sv2 = new static_var<int>();

	auto sva1 = new static_var<int[]>();
	sva1->resize(5);
	auto sva2 = new static_var<int[]>();
	sva2->resize(5);


	dyn_var<int> y;

	delete sv1;
	delete sv2;
	delete sva1;
	delete sva2;
	
	// A realistic test is hard to get right (depends on rv-optimizations)
	// so we will just stick with this simple one above

}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

#include "blocks/c_code_generator.h"
#include "builder/array.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"

using builder::dyn_arr;
using builder::dyn_var;
using builder::static_var;

static void foo(void) {
	dyn_var<int> x;
	dyn_arr<int, 2> y = {0, 0};
	y[0] = 1;
	while (1)
		dyn_arr<int, 2> z = {1, 2};
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(foo, "my_bar");
	ast->dump(std::cout, 0);

	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

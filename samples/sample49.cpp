#include "blocks/c_code_generator.h"
#include "builder/dyn_var.h"

using builder::dyn_var;

static void foo(void) {
	std::string name = "my_var";
	dyn_var<int> v = builder::with_name(name, true);
	v = 1;
	dyn_var<int> v1 = builder::with_name(name);
	dyn_var<int> y = v1;
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(foo, "foo");
	ast->dump(std::cout, 0);

	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

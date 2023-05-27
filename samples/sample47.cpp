#include "blocks/c_code_generator.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"

using builder::dyn_var;
using builder::static_var;

static void foo(void) {
	static_var<int[]> states;
	states.resize(4);
	for (states[0] = 0; states[0] < 2; states[0]++) {
		for (states[1] = 0; states[1] < 2; states[1]++) {
			for (states[2] = 0; states[2] < 2; states[2]++) {
				for (states[3] = 0; states[3] < 2; states[3]++) {
					dyn_var<int> c = 0;
				}
			}
		}
	}
}
int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(foo, "my_bar");
	ast->dump(std::cout, 0);

	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

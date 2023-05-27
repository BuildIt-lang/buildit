/*NO_TEST*/
#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/builder_union.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::builder_union;
using builder::dyn_var;
using builder::static_var;

static void foo(void) {
	dyn_var<int> x;
	dyn_var<int> y;
	builder_union<int> z = x + y;
	builder_union<int> res = 1;
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

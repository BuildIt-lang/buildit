#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include "blocks/rce.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;

dyn_var<int(void)> create_int = builder::as_global("create_int");

static void bar(void) {
	dyn_var<int> x = create_int();
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	block::eliminate_redundant_vars(ast);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

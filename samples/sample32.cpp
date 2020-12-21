#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;

const char graph_t_name[] = "GraphT";
using graph_t = typename builder::name<graph_t_name>;

static void bar(void) {
	dyn_var<graph_t> g;
	g = g + 1;
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(bar);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

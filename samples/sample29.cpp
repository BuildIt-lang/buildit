#include "blocks/c_code_generator.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;

// Don't declare this as constexpr, so that graph_t 
// has external linkage and operator can left undefined
char graph_t_name[] = "GraphT";
using graph_t = typename builder::name<graph_t_name>;

extern char foo_t_name[];
char foo_t_name[] = "FooT";

template <typename T>
using foo_t = typename builder::name<foo_t_name, T>;

// Declare the operators inside the builder namespace
// this allows ADL to find the operators. Otherwise atleast clang
// doesn't see this
namespace builder {
graph_t operator + (const graph_t& a, const int&);
template <typename T>
foo_t<T> operator + (const foo_t<T>& a, const int&);
}

static void bar(void) {
	dyn_var<graph_t> g;
	g = g + 1;

	dyn_var<foo_t<int>> f;
	f = f + 1;
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(bar);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

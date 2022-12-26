#include "blocks/matchers/patterns.h"
#include "blocks/matchers/matchers.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"

using namespace block::matcher;
using builder::dyn_var;
using builder::static_var;

static void foo(void) {
	dyn_var<int> x = 0;	
	x = x + 1;
	// this should not be matched
	dyn_var<int> y = 0;
	y = x + 1;
	dyn_var<int> z = 0;
	z = z + 2;
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(foo, "foo");

	auto p = assign_expr(var("a"), binary_expr(var("a"), int_const(1)));
	auto matches = find_all_matches(p, ast);
	std::cout << "Found " << matches.size() << " matches" << std::endl;
	std::cout << "-----" << std::endl;
	for (auto x: matches) {
		x.node->dump(std::cout, 0);
		std::cout << "-----" << std::endl;
	}
	return 0;
}

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
	dyn_var<int> sum = 0;
	const int arr[] = {1, 3, 4, 0, 2, 6, 0, 8, 0, 0, 1, -2, 0, 0, 3};

	for (static_var<unsigned int> x = 0; x < sizeof(arr) / sizeof(*arr); x++) {
		if (arr[x] != 0) {
			builder::annotate("roll.0");
			sum = sum + arr[x];
		}
	}

	const int adj[5][5] = {{4, 0, 1, 0, 0}, {5, 0, 0, 0, 1}, {0, 0, 1, 0, 0}, {1, 2, 1, 0, 0}, {0, 0, 0, 0, 0}};

	dyn_var<int[5]> old_ranks;
	dyn_var<int[5]> new_ranks;

	for (static_var<int> src = 0; src < 5; src++) {
		// dyn_var<int> sum = 0;
		for (static_var<int> dst = 0; dst < 5; dst++) {
			if (adj[src][dst] != 0) {
				builder::annotate("roll.1");
				// builder::annotate("roll.1." +
				// std::to_string(src));
				new_ranks[src] = new_ranks[src] + adj[src][dst] * old_ranks[dst];
			}
		}
	}
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

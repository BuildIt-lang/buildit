#include "blocks/c_code_generator.h"
#include "blocks/rce.h"
#include "builder/dyn_var.h"

using builder::dyn_var;
using namespace std;

static dyn_var<int> bar(dyn_var<int> n) {
	if (n) {
	}
	return 0;
}

static void staged() {
	bar(24) != 1 && bar(32) != 1;
}

static void staged2() {
	dyn_var<int> a = 0;
	for (dyn_var<int> i = 0; i < 5; i = i + 1) {
		a = a + 1;
	}
	dyn_var<int> b = a;
	for (dyn_var<int> i = 0; i < a; i = i + 1) {
		b = b + 1;
	}
}

int main() {
	// This example causes a segfault on block::binary_is_same<block::and_expr>.
	// If you only use a single condition within the while loop, it's fine.
	if (true) {
		auto ast = builder::builder_context().extract_function_ast(staged, "staged");
		block::c_code_generator::generate_code(ast, cout, 0);
	}

	// This shows different semantics before and after running rce.
	if (true) {
		auto ast = builder::builder_context().extract_function_ast(staged2, "staged2");
		block::c_code_generator::generate_code(ast, cout, 0);
		block::eliminate_redundant_vars(ast);
		block::c_code_generator::generate_code(ast, cout, 0);
	}
}

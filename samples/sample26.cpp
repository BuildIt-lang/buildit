#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;

static void bar(void) {
	dyn_var<int> k = 0;
	dyn_var<int> t = 0;
	/*
		for (static_var<int> i = 0; i < 2; i++) {
			if (k == i) {
				for (dyn_var<int> s = 0; s < i; s = s + 1) {
					t = t + 1;
				}
			}
		}
	*/
	while (1) {
		if (k == 0)
			break;
		t = t + 1;
		t = t + 1;
		while (1) {
			if (k == 0)
				break;
		}

		t = t + 1;
		while (1) {
			if (k == 0)
				break;
			t = t + 1;
		}
		t = t + 1;
	}
	t = t + 1;
	while (1) {
		if (k == 0)
			break;
		t = t + 1;
	}
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(bar);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

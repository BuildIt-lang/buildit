// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include "blocks/rce.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static void bar(void) {
	static_var<std::vector<int>> x;
	while (x.val.size() < 3) {
		dyn_var<int> y = x.val.size();
		if (y < x.val.size()) {
			y++;
		} else {
			y--;
		}
		x.val.push_back(2);
	}
} 

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}



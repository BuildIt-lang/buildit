// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include "blocks/rce.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

struct linked_list {
	static constexpr const char* type_name = "linked_list";
	dyn_var<linked_list*> next = builder::with_name("next");
};

static void bar(void) {
	dyn_var<struct linked_list*> x;
	x->next->next->next = 0;
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}



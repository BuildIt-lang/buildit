// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include "blocks/rce.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

struct foo;

// Mechanism for assigning names to incomplete types
namespace builder {
template <>
struct external_type_namer<foo> {
	static constexpr const char* type_name = "struct foo";
};
}

struct linked_list {
	static constexpr const char* type_name = "linked_list";
	dyn_var<linked_list*> next = builder::with_name("next");
	dyn_var<foo*> other = builder::with_name("other");
};

static void bar(void) {
	dyn_var<struct linked_list*> x;
	x->next->next->next = 0;
	x->other = 0;
}

int main(int argc, char* argv[]) {
	block::c_code_generator::generate_struct_decl<dyn_var<linked_list>>(std::cout);
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}



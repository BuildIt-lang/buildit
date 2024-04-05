#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

using builder::as_member;
using builder::dyn_var;
using builder::static_var;


struct struct_type {
	dyn_var<int> x;
	dyn_var<float> y;
};

struct my_type {
	static constexpr const char* type_name = "my_type";
	dyn_var<struct_type> nested = builder::with_name("nested");
	dyn_var<int> another;
};

static void bar(void) {
	dyn_var<my_type> a;
	dyn_var<struct_type> b;
	
	a.nested = b;
	a.nested.x = a.another;
	
	a.nested.y++;
}

int main(int argc, char *argv[]) {

	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	ast->dump(std::cout, 0);

	block::c_code_generator::generate_struct_decl<dyn_var<struct_type>>(std::cout);
	block::c_code_generator::generate_struct_decl<dyn_var<my_type>>(std::cout);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

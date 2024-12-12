#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include "builder/lib/utils.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;


template <typename T>
struct vector: public builder::custom_type<T> {
	static constexpr const char* type_name = "std::vector";
	typedef T dereference_type;
	dyn_var<void(int)> resize = builder::with_name("resize");
};

static void bar(void) {
	dyn_var<vector<vector<int>>> x;
	x.resize(2);
	x[0].resize(1);
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

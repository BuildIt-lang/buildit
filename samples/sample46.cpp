#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include <iostream>
using builder::dyn_var;
using builder::static_var;
using builder::as_member_of;



struct FooT: public builder::custom_type_base {
	static constexpr const char* type_name = "FooT";
	dyn_var<int> member = as_member_of(this, "member");
};

template <typename T>
struct BarT: public builder::custom_type_base {
	static constexpr const char* type_name = "BarT";
	dyn_var<int> my_member = as_member_of(this, "my_member");
	dyn_var<int> second_member = as_member_of(this, "second_member");
};

template <typename T1, typename T2>
struct CarT: public builder::custom_type_base {
	static constexpr const char* type_name = "CarT";
	dyn_var<int> my_member = as_member_of(this, "my_member");
};

static void bar(void) {
	dyn_var<FooT> g;
	g = g + 1;
	dyn_var<int> x = g.member;
	dyn_var<FooT> h = g;
	h = g;

	dyn_var<BarT<int>> f;
	f.second_member = g + 1;

	dyn_var<CarT<int, BarT<float>>> l;
	l.my_member = g.member;
}



int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(bar);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}
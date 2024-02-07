#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::as_member;
using builder::dyn_var;
using builder::static_var;

struct FooT : public builder::custom_type<> {
	static constexpr const char *type_name = "FooT";
	dyn_var<int> member = as_member("member");
};

template <typename T>
struct BarT : public builder::custom_type<> {
	static constexpr const char *type_name = "BarT";
	dyn_var<T> my_member = as_member("my_member");
	dyn_var<T> second_member = as_member("second_member");
};

template <typename T1, typename T2>
struct CarT : public builder::custom_type<T1, T2> {
	static constexpr const char *type_name = "CarT";
	dyn_var<T2> my_member = as_member("my_member");
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

	dyn_var<FooT *> i;
	i->member = l.my_member;
	(*i).member = 0;
	i[3].member = 0;
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "my_bar");
	ast->dump(std::cout, 0);

	block::c_code_generator::generate_struct_decl<dyn_var<FooT>>(std::cout);
	block::c_code_generator::generate_struct_decl<dyn_var<BarT<int>>>(std::cout);

	// Don't do this because this has a template in the generated type
	// block::c_code_generator::generate_struct_decl<dyn_var<CarT<int, BarT<float>>>>(std::cout);

	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

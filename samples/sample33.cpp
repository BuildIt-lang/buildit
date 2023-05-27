#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::as_member;
using builder::dyn_var;
using builder::static_var;

constexpr char foo_t_name[] = "FooT";
using foo_t = typename builder::name<foo_t_name>;

class FooT : public dyn_var<foo_t> {
public:
	typedef dyn_var<foo_t> super;
	using super::super;
	using dyn_var<foo_t>::operator=;
	FooT(const FooT &t) : dyn_var<foo_t>((builder::builder)t) {}
	FooT() : dyn_var<foo_t>() {}
	builder::builder operator=(const FooT &t) {
		return (*this) = (builder::builder)t;
	}

	dyn_var<int> member = as_member(this, "member");
};

static void bar(void) {
	FooT g;
	g = g + 1;
	dyn_var<int> x = g.member;
	FooT h = g;
	h = g;
	dyn_var<foo_t *> ptr = &g;
	((FooT)(builder::cast)g[0]).member = 0;
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(bar);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
#include <memory>
using builder::as_member;
using builder::dyn_var;
using builder::static_var;

constexpr char foo_t_name[] = "FooT";
using foo_t = typename builder::name<foo_t_name>;

namespace builder {

// Use specialization instead of inheritance
template <>
class dyn_var<foo_t> : public dyn_var_impl<foo_t> {
public:
	typedef dyn_var_impl<foo_t> super;
	using super::super;
	using super::operator=;
	builder operator=(const dyn_var<foo_t> &t) {
		return (*this) = (builder)t;
	}
	dyn_var(const dyn_var &t) : dyn_var_impl((builder)t) {}
	dyn_var() : dyn_var_impl<foo_t>() {}

	dyn_var<int> member = as_member(this, "member");
};

/* Specialization for foo_t* is not required because it comes built in with dyn_var now */

} // namespace builder

static void bar(void) {
	dyn_var<foo_t> g;
	g = g + 1;
	dyn_var<int> x = g.member;
	dyn_var<foo_t> h = g;
	h = g;
	dyn_var<foo_t *> ptr = &g;
	(*ptr).member = 0;
	ptr->member = 1;

	// This SHOULD produce a copy
	dyn_var<foo_t> i = *ptr;
	i.member = 3;
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(bar);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

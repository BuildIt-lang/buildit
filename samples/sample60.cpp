// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include "blocks/rce.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;
using builder::generic;
using builder::type_of;
using builder::with_type;


static dyn_var<generic> get_max(dyn_var<generic> x, dyn_var<generic> y) {

	if (type_of(x) != type_of(y)) {
		assert(false && "get_max can only compare similar types");
	}

	dyn_var<generic> res = with_type(type_of(x));
	if (x < y) res = y;
	else res = x;
	return res;
}

static void set_zero(dyn_var<generic> ptr) {
	*ptr = 0;
}

static void bar(void) {
	dyn_var<generic> x;
	x.set_type(builder::create_type<int>());

	dyn_var<generic> y = with_type(builder::create_type<long>());	

	dyn_var<int> a, b;
	dyn_var<float> m,n;

	dyn_var<int> m1 = get_max(with_type(a), with_type(b));
	dyn_var<float> m2 = get_max(m, n);


	set_zero(with_type(pointer_of(type_of(x)), &x));
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	context.run_rce = true;
	auto ast = context.extract_function_ast(bar, "bar");
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}



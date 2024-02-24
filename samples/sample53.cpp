// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/dyn_var.h"
#include "builder/lib/utils.h"
#include "builder/static_var.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

struct external_object_t {
	dyn_var<int> member = builder::defer_init();
	static_var<int> counter = builder::defer_init();
};

static void foo(external_object_t &obj) {
	// Init not
	obj.member.deferred_init(builder::with_name("member_var", true));
	obj.counter.deferred_init();

	dyn_var<int> x = 0;
	if (x) {
		obj.member = 1;
	} else {
		obj.member = 2;
	}

	for (obj.counter = 0; obj.counter < 10; obj.counter++)
		if (obj.member % 2 == 0)
			obj.member += obj.counter;
}

int main(int argc, char *argv[]) {

	external_object_t obj;

	builder::builder_context context;
	block::c_code_generator::generate_code(context.extract_function_ast(foo, "foo", obj), std::cout, 0);
	return 0;
}

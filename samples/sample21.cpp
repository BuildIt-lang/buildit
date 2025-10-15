#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

using builder::dyn_var;
using builder::static_var;


// We are now deprecating foreign_exprs 
class dummy {
public:
	std::string value;

	// We have to define this so that the foreign_expr can check for
	// equality
	bool operator==(dummy &other) {
		if (value == other.value)
			return true;
		return false;
	}

	// Operator builder::builder is not defined anymore
};

// A simple straight line code with 2 variable declarations and one operator
int main(int argc, char *argv[]) {
	builder::builder_context context;

	dummy foo;
	foo.value = "foo";

	auto ast = context.extract_ast_from_function([=] {
		dyn_var<int> x = 2;
		// Since foreign functions are deprecated, use the value from foo
		dyn_var<int> bar = foo.value + x;
	});

	ast->dump(std::cout, 0);
	return 0;
}

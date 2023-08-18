#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/builder_union.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
using builder::builder_union;
using builder::dyn_var;
using builder::static_var;

static void foo(void) {
	dyn_var<short int> a;
	dyn_var<unsigned short int> b;
	dyn_var<int> c;
	dyn_var<unsigned int> d;
	dyn_var<long> e;
	dyn_var<unsigned long> f;
	dyn_var<long long> g;
	dyn_var<unsigned long long> h = (unsigned long long)4;
	dyn_var<char> i;
	dyn_var<unsigned char> j;
	dyn_var<float> k;
	dyn_var<double> l;
	dyn_var<void *> m;
	dyn_var<char[]> n = "Hello world";
	n = "new string";

	// bool test, fixes a bug
	// that causes false as an init value creates a variable
	// without context
	dyn_var<int> x = false;
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

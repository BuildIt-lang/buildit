#include "builder/dyn_var.h"
#include "blocks/c_code_generator.h"
#include "builder/array.h"

using builder::dyn_var;
using builder::dyn_arr;
using namespace std;

static void foo() {
	dyn_arr<int, 3> x;
	x[0] = 1;
	x[1] = x[0] + 2;
	x[2] = x[1] + x[0];
	
	dyn_arr<int, 4> y = {0, x[0] + 4, 0, 0};

	dyn_arr<int> z = {1, 2};
	
	dyn_arr<int> a;
	a.set_size(2);


	dyn_arr<int> b = y;
	dyn_arr<int, 5> c = a;
	
}
int main(int argc, char* argv[]) {
	auto ast = builder::builder_context().extract_function_ast(foo, "foo");
	block::c_code_generator::generate_code(ast, cout, 0);
	return 0;
}

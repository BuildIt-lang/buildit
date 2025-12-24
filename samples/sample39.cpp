#include "blocks/c_code_generator.h"
#include "builder/array.h"
#include "builder/dyn_var.h"

using builder::dyn_arr;
using builder::dyn_var;
using builder::arr;

using namespace std;



struct container {
	dyn_var<int> idx;
	dyn_var<int*> next;

	container() {
		idx = 0;
	}
};


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


	arr<container, 5> containers;
	containers[0].next = &(containers[1].idx);


	arr<container, 2> conts = {containers[0], containers[1]};

}
int main(int argc, char *argv[]) {
	auto ast = builder::builder_context().extract_function_ast(foo, "foo");
	block::c_code_generator::generate_code(ast, cout, 0);
	return 0;
}

/*NO_TEST*/
// Include the headers
#include "blocks/c_code_generator.h"
#include "blocks/extract_cuda.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>
template <typename T>
void resize(T &t, int size) {
	block::to<block::array_type>(t.block_var->var_type)->size = size;
}

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static dyn_var<int> choose(dyn_var<int> n, dyn_var<int> k, const int MAX_N) {
	int comp[MAX_N][MAX_N];
	for (int i = 0; i < MAX_N; i++) {
		comp[i][0] = 1;
		comp[i][i] = 1;
	}
	for (int i = 1; i < MAX_N; ++i) {
		comp[0][i] = 0;
		for (int j = 1; j < i; ++j) {
			comp[i][j] = comp[i - 1][j - 1] + comp[i - 1][j];
			comp[j][i] = 0;
		}
	}
	dyn_var<int[]> comp_r;
	resize(comp_r, MAX_N * MAX_N);

	for (static_var<int> i = 0; i < MAX_N * MAX_N; i++) {
		builder::annotate("roll.0");
		comp_r[i] = comp[i / MAX_N][i % MAX_N];
	}
	return comp_r[n * MAX_N + k];
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(choose, "choose", 10);

	std::cout << "#include <iostream>" << std::endl;
	block::c_code_generator::generate_code(ast, std::cout, 0);
	std::cout << "int main(int argc, char* argv[]) {\n\tstd::cout << choose(8, 3) << std::endl;\n}" << std::endl;
}

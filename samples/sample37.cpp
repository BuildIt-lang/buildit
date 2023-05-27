// Include the headers
#include "blocks/c_code_generator.h"
#include "blocks/extract_cuda.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;
static void bar(dyn_var<int *> buffer) {
	builder::annotate(CUDA_KERNEL);
	for (dyn_var<int> cta = 0; cta < 128; cta = cta + 1) {
		for (dyn_var<int> tid = 0; tid < 512; tid = tid + 1) {
			dyn_var<int> thread_id = cta * 512 + tid;
			buffer[thread_id] = 0;
		}
	}
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	auto new_decls = block::extract_cuda_from(block::to<block::func_decl>(ast)->body);
	for (auto a : new_decls)
		block::c_code_generator::generate_code(a, std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
}

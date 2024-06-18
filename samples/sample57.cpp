// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include "blocks/rce.h"
#include <iostream>

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

static void bar(void) {
     // Insert code to stage here
    
    dyn_var<int> x = 0;
    dyn_var<int*> y = &x;
    
    (*y)++;

    dyn_var<int> z = 0;
    z++;

    dyn_var<int> a = z;
    z++;

    dyn_var<int> b = a;
    b++;
     
     
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_function_ast(bar, "bar");
	block::eliminate_redundant_vars(ast);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}



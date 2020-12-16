#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include <iostream>

// We will now created our own builder types and dyn_var types using CRTP

class my_builder: public builder::builder_base<my_builder> {
public:
	using builder::builder_base<my_builder>::builder_base;
	my_builder operator=(const my_builder &a) {
		return assign(a);
	}
	using builder::builder_base<my_builder>::operator[];
};

template <typename T>
class my_dyn_var : public builder::dyn_var_base<T, my_dyn_var<T>, my_builder> {
public:
	virtual ~my_dyn_var() = default;
	using builder::dyn_var_base<T, my_dyn_var<T>, my_builder>::dyn_var_base;

	my_builder operator=(const my_dyn_var<T> &a) {
		return (*this = (my_builder)a);
	}
	using builder::dyn_var_base<T, my_dyn_var<T>, my_builder>::operator[];
	using builder::dyn_var_base<T, my_dyn_var<T>, my_builder>::operator=;

};




int main(int argc, char *argv[]) {
	builder::builder_context context;

	auto ast = context.extract_function_ast(
	    [=](my_dyn_var<int> x) -> my_dyn_var<int> {
		    my_dyn_var<int> z = 3 + x;
		    return z + 2;
	    },
	    "func1");
	ast->dump(std::cout, 0);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

#include "blocks/c_code_generator.h"
#include "builder/builder_context.h"
#include "builder/builder_dynamic.h"
#include "builder/dyn_var.h"
#include <set>
using builder::dyn_var;

dyn_var<int(char *)> myprintf = builder::as_global("printf");

static void foo(dyn_var<int> count, char to_print, std::set<char> &working_set, std::set<char> &done_set) {
	for (dyn_var<int> i = 0; i < count; i++) {
		myprintf("%c", to_print);
	}
	myprintf("\\n");
	if (to_print < 'z') {
		if (done_set.find(to_print + 1) == done_set.end())
			working_set.insert(to_print + 1);
		std::string name = "print_" + std::to_string(to_print + 1);
		dyn_var<void(int)> bar = builder::with_name(name);
		bar(count + 1);
	}
}

int main(int argc, char *argv[]) {
	std::set<char> working_set, done_set;
	std::vector<block::block::Ptr> functions;

	working_set.insert('a');

	while (!working_set.empty()) {
		char c = *working_set.begin();
		working_set.erase(c);
		std::string name = "print_" + std::to_string(c);
		auto ast = builder::builder_context().extract_function_ast(foo, name, c, working_set, done_set);
		functions.push_back(ast);
		done_set.insert(c);
	}

	builder::builder_context ctx;
	ctx.dynamic_header_includes = "#include <stdio.h>";
	void (*fptr)(int) = (void (*)(int))builder::compile_asts(ctx, functions, "print_" + std::to_string('a'));

	fptr(1);

	return 0;
}

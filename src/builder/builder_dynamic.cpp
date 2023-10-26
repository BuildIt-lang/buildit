#include "builder/builder_dynamic.h"

namespace builder {
std::string get_temp_filename(builder_context &context) {
	char base_name_c[] = GEN_TEMPLATE_NAME;
	int fd = mkstemp(base_name_c);
	if (fd < 0) {
		assert(false && "Opening a temporary file failed\n");
	}

	close(fd);

	std::string base_name(base_name_c);
	std::string source_name = base_name;

	if (!context.dynamic_use_cxx)
		source_name += ".c";
	else
		source_name += ".cpp";

	return source_name;
}

void *compile_and_return_ptr(builder_context &context, std::string source_name, std::string fname) {
	std::string compiled_name = source_name + ".so";

	std::string compiler_name;

	if (!context.dynamic_use_cxx)
		compiler_name = COMPILER_PATH;
	else {
		compiler_name = CXX_COMPILER_PATH;
		compiler_name += " -std=c++11 -fPIC ";
	}

	std::string compile_command = compiler_name + " -shared -O3 " + source_name + " -o " + compiled_name + " " +
				      context.dynamic_compiler_flags;

	int err = system(compile_command.c_str());
	if (err != 0) {
		assert(false && "Compilation failed\n");
	}

	void *handle = dlopen(compiled_name.c_str(), RTLD_NOW | RTLD_LOCAL);
	if (!handle) {
		assert(false && "Loading compiled module failed\n");
	}

	void *function = dlsym(handle, fname.c_str());
	if (!function) {
		assert(false && "Loading compiled module failed\n");
	}
	return function;
}

void *compile_asts(builder_context context, std::vector<block::block::Ptr> asts, std::string lookup_name) {
	std::string source_name = get_temp_filename(context);
	std::ofstream oss(source_name);
	oss << context.dynamic_header_includes << std::endl;

	if (context.dynamic_use_cxx)
		oss << "extern \"C\" { " << std::endl;

	// First spit out all the declarations
	for (auto x : asts) {
		block::c_code_generator::generate_code(x, oss, 0, true);
	}
	// Then the actual implementations
	for (auto x : asts) {
		block::c_code_generator::generate_code(x, oss, 0);
	}

	if (context.dynamic_use_cxx)
		oss << "}" << std::endl; // For the extern "C"
	oss.close();

	return compile_and_return_ptr(context, source_name, lookup_name);
}

} // namespace builder

#ifndef BUILDER_DYNAMIC_H
#define BUILDER_DYNAMIC_H

#include "builder/builder_context.h"
#include "blocks/c_code_generator.h"
#include "blocks/rce.h"
#include <cstdio>
#include <fstream>
#include <unistd.h>
#include <dlfcn.h>

#include "gen/compiler_headers.h"

namespace builder {

template <typename FT, typename...ArgsT>
auto compile_function_with_context(builder_context context, FT f, ArgsT...args) -> void* {

	auto ast = context.extract_function_ast(f, "execute", args...);
	// Proactively run RCE
	if (context.run_rce)
		block::eliminate_redundant_vars(ast);		

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

	std::string compiled_name = base_name + ".so";
	
	std::string compiler_name;

	if (!context.dynamic_use_cxx) 	
		compiler_name = COMPILER_PATH;
	else {
		compiler_name = CXX_COMPILER_PATH;
		compiler_name += " -std=c++11";
	}

	std::string compile_command = compiler_name + " -shared -O3 " + source_name + " -o " + compiled_name;
	
		
	std::ofstream oss(source_name);	

	oss << context.dynamic_header_includes << std::endl;

	if (context.dynamic_use_cxx) 
		oss << "extern \"C\" ";

		
	block::c_code_generator::generate_code(ast, oss, 0);

	oss.close();

	int err = system(compile_command.c_str());
	if (err != 0) {	
		assert(false && "Compilation failed\n");
	}
	
	void* handle = dlopen(compiled_name.c_str(), RTLD_NOW | RTLD_LOCAL);
	if (!handle) {
		assert(false && "Loading compiled module failed\n");
	}
	
	void* function = dlsym(handle, "execute");
	if (!function) {
		assert(false && "Loading compiled module failed\n");
	}
	return function;
}

template <typename FT, typename...ArgsT>
auto compile_function(FT f, ArgsT...args) -> void* {
	// Use an unconfigured context object
	builder_context context;
	return compile_function_with_context(context, f, args...);
}

}
#endif

#ifndef BUILDER_DYNAMIC_H
#define BUILDER_DYNAMIC_H

#include "blocks/c_code_generator.h"
#include "blocks/rce.h"
#include "builder/builder_context.h"
#include <cstdio>
#include <dlfcn.h>
#include <fstream>
#include <unistd.h>

#include "gen/compiler_headers.h"

namespace builder {

std::string get_temp_filename(builder_context &context);
void *compile_and_return_ptr(builder_context &context, std::string source_name, std::string fname);

// Interface to just compile ASTs(s)
void *compile_asts(builder_context context, std::vector<block::block::Ptr> asts, std::string lookup_name);

template <typename FT, typename... ArgsT>
auto compile_function_with_context(builder_context context, FT f, ArgsT... args) -> void * {

	auto ast = context.extract_function_ast(f, "execute", args...);
	// Proactively run RCE
	if (context.run_rce)
		block::eliminate_redundant_vars(ast);

	std::string source_name = get_temp_filename(context);

	std::ofstream oss(source_name);

	oss << context.dynamic_header_includes << std::endl;

	if (context.dynamic_use_cxx)
		oss << "extern \"C\" ";

	block::c_code_generator::generate_code(ast, oss, 0);

	oss.close();

	return compile_and_return_ptr(context, source_name, "execute");
}

template <typename FT, typename... ArgsT>
auto compile_function(FT f, ArgsT... args) -> void * {
	// Use an unconfigured context object
	builder_context context;
	return compile_function_with_context(context, f, args...);
}

} // namespace builder
#endif

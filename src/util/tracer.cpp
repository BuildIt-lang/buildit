#include "util/tracer.h"
#include <string>
#include "builder/builder_context.h"

namespace tracer {

tag get_offset_in_function_impl(ast_function_type _function, builder::builder_context *current_builder_context) {
	unsigned long long function = (unsigned long long) (void*) _function;
	void *buffer[20];
	tag new_tag;
	// First add the RIP pointers
	int backtrace_size = backtrace(buffer, 20);	
	char** backtrace_functions = backtrace_symbols(buffer, backtrace_size);
	for (int i = 0; i < backtrace_size; i++) {
		int offset;
		unsigned long long address;
		if (sscanf(backtrace_functions[i], "%*[^+]+%x) [%llx]", &offset, &address) != 2)
			continue;
		new_tag.pointers.push_back(address);
		if (function == address - offset)
			break;
	}
	free (backtrace_functions);

	// Now add snapshots of static vars
	assert(current_builder_context != nullptr);
	for (builder::tracking_tuple tuple: current_builder_context->static_var_tuples) {
		new_tag.static_var_snapshots.push_back(tuple.snapshot());
	}
	return new_tag;
}
}

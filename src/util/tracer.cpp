#include "util/tracer.h"
#include <string>
#include "builder/builder_context.h"

namespace tracer {
static unsigned long long call_point = 0;

void set_call_point(ast_function_type _function) {
	unsigned long long function = (unsigned long long) (void*) _function;
	void *buffer[20];
	int backtrace_size = backtrace(buffer, 20);	
	char** backtrace_functions = backtrace_symbols(buffer, backtrace_size);
	int i;
	for (i = 0; i < backtrace_size; i++) {
		int offset;
		unsigned long long address;
#ifdef __linux
		if (sscanf(backtrace_functions[i], "%*[^+]+%x) [%llx]", &offset, &address) != 2)
			continue;
#elif __APPLE__
		if (sscanf(backtrace_functions[i], "%*[^+]+ %i", &offset) != 1)
			continue;
		address = (unsigned long long)buffer[i];
#else
		#error Backtracer currently only supported for Linux and MacOS
#endif
		if (function == address - offset)
			break;	
	}
	call_point = (unsigned long long)buffer[i + 1];
	free (backtrace_functions);	
}
tag get_offset_in_function_impl(ast_function_type _function, builder::builder_context *current_builder_context) {
	if (call_point == 0) {
		set_call_point(_function);
	}
	void *buffer[20];
	tag new_tag;
	// First add the RIP pointers
	int backtrace_size = backtrace(buffer, 20);	
	for (int i = 0; i < backtrace_size; i++) {
		if ((unsigned long long)buffer[i] == call_point)
			break;
		new_tag.pointers.push_back((unsigned long long)buffer[i]);
	}

	// Now add snapshots of static vars
	assert(current_builder_context != nullptr);
	for (builder::tracking_tuple tuple: current_builder_context->static_var_tuples) {
		new_tag.static_var_snapshots.push_back(tuple.snapshot());
	}
	return new_tag;
}
}

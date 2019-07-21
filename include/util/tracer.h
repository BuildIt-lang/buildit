#ifndef TRACER_H
#define TRACER_H
#include <execinfo.h>

namespace builder {
class builder_context;
}
typedef void (*ast_function_type) (builder::builder_context* context);
static int get_offset_in_function(ast_function_type _function) {
	void * function = (void*) _function;
	void *buffer[20];
	int backtrace_size = backtrace(buffer, 20);
	char** backtrace_functions = backtrace_symbols(buffer, backtrace_size);
	
	for (int i = 0; i < backtrace_size; i++) {
		int offset;
		unsigned long long address;
		char *start = backtrace_functions[i];
		while (*start && *start != '+')
			start++;
		if (*start != '+')
			continue;
		sscanf(start, "%x) [%llx]", &offset, &address);
		if ((unsigned long long) function == address - offset) {
			free (backtrace_functions);
			return offset;
		}
	} 
	free (backtrace_functions);
	assert(false);
	return -1;
}


#endif

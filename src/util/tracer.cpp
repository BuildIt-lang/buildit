#include "util/tracer.h"
#include "builder/builder_context.h"
#include <string>
namespace builder {
extern void lambda_wrapper(void);
}
namespace tracer {
static unsigned long long call_point = 0;

static void set_call_point(void) {
	unsigned long long function = (unsigned long long)(void *)builder::lambda_wrapper;
#ifdef TRACER_USE_FLIMITS
	unsigned long long function_end = (unsigned long long)(void *)builder::lambda_wrapper_close;
#endif
	void *buffer[20];
	int backtrace_size = backtrace(buffer, 20);
	char **backtrace_functions = backtrace_symbols(buffer, backtrace_size);
	int i;
	bool found = false;
	for (i = 0; i < backtrace_size; i++) {
		unsigned long long address;
#ifndef TRACER_USE_FLIMITS
		unsigned int offset;
#ifdef __linux
		if (sscanf(backtrace_functions[i], "%*[^+]+%x) [%llx]", &offset,
			   &address) != 2) {
			// printf("Scanning of backtrace failed, result might be
			// bad\n");
			continue;
		}
#elif __APPLE__
		if (sscanf(backtrace_functions[i], "%*[^+]+ %i", &offset) !=
		    1) {
			// printf("Scanning of backtrace failed, result might be
			// bad\n");
			continue;
		}
		address = (unsigned long long)buffer[i];
#else
#error Backtracer currently only supported for Linux and MacOS
#endif

		if (function == address - offset) {
			found = true;
			break;
		}
#else
		address = (unsigned long long)buffer[i];
		
		if (function <= address && address < function_end) {
			found = true;
			break;
		}
			
#endif
	}
	if (!found)
		assert(false && "Call point not found\n");
	call_point = (unsigned long long)buffer[i + 1];
	free(backtrace_functions);
}
tag get_offset_in_function_impl(
    builder::builder_context *current_builder_context) {
	if (call_point == 0) {
		set_call_point();
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
	for (builder::tracking_tuple tuple :
	     current_builder_context->static_var_tuples) {
		new_tag.static_var_snapshots.push_back(tuple.snapshot());
	}
	return new_tag;
}

static unsigned long long unique_tag_counter = 0;
tag get_unique_tag(void) {
	tag new_tag;
	new_tag.pointers.push_back(0);
	new_tag.pointers.push_back(unique_tag_counter);
	unique_tag_counter++;
	return new_tag;
}

} // namespace tracer

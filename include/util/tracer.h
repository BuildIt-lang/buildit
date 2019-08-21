#ifndef TRACER_H
#define TRACER_H
#include <execinfo.h>
#include <vector>

namespace builder {
class builder_context;
}

namespace tracer {

class tag {
public:
	std::vector<unsigned long long> pointers;
	bool operator== (const tag &other) {
		if (other.pointers.size() != pointers.size())
			return false;
		for (int i = 0; i < pointers.size(); i++) 
			if (pointers[i] != other.pointers[i])
				return false;
		return true;
	}	
	bool operator != (const tag &other) {
		return !operator == (other);
	}
	bool is_empty(void) {
		return pointers.size() == 0;
	}
	void clear(void) {
		pointers.clear();
	}
	std::string stringify(void) {
		std::string output_string = "[";
		for (int i = 0; i < pointers.size(); i++) {
			char temp[128];
			sprintf(temp, "%llx", pointers[i]);
			output_string += temp;
			if (i != pointers.size() - 1)
				output_string += ", ";
		}
		output_string += "]";
		return output_string;
	}
};
typedef void (*ast_function_type) (void);


static tag get_offset_in_function_impl(ast_function_type _function) {
	unsigned long long function = (unsigned long long) (void*) _function;
	void *buffer[20];
	tag new_tag;
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
	return new_tag;
}
static int get_offset_in_function_impl_old(ast_function_type _function) {
	void * function = (void*) _function;
	void *buffer[20];
	int backtrace_size = backtrace(buffer, 20);
	char** backtrace_functions = backtrace_symbols(buffer, backtrace_size);

	for (int i = 0; i < backtrace_size; i++) {
		int offset;
		unsigned long long address;
		if (sscanf(backtrace_functions[i], "%*[^+]+%x) [%llx]", &offset, &address) != 2)
			continue;
		if ((unsigned long long) function == address - offset) {
			free (backtrace_functions);
			return offset;
		}
	} 
	free (backtrace_functions);
	assert(false);
	return -1;
}

}
#endif

#ifndef BUILDER_EXCEPTIONS
#define BUILDER_EXCEPTIONS
#include <exception>

namespace builder{
struct OutOfBoolsException: public std::exception {
	OutOfBoolsException(int offset): static_offset(offset) {}
	int32_t static_offset;	
};
}

#endif

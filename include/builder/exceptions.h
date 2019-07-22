#ifndef BUILDER_EXCEPTIONS
#define BUILDER_EXCEPTIONS
#include <exception>

namespace builder{
struct OutOfBoolsException: public std::exception {
	OutOfBoolsException(int32_t offset): static_offset(offset) {}
	int32_t static_offset;	
};
struct LoopBackException: public std::exception {
	LoopBackException(int32_t offset): static_offset(offset) {}
	int32_t static_offset;
};
}

#endif

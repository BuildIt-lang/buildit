#ifndef BUILDER_EXCEPTIONS
#define BUILDER_EXCEPTIONS
#include <exception>
#include "util/tracer.h"

namespace builder{
struct OutOfBoolsException: public std::exception {
	OutOfBoolsException(tracer::tag offset): static_offset(offset) {}
	tracer::tag static_offset;	
};
struct LoopBackException: public std::exception {
	LoopBackException(tracer::tag offset): static_offset(offset) {}
	tracer::tag static_offset;
};
}

#endif

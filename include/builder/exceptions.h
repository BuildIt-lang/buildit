#ifndef BUILDER_EXCEPTIONS
#define BUILDER_EXCEPTIONS
#include <exception>
#include "util/tracer.h"
#include "blocks/stmt.h"

namespace builder{
struct OutOfBoolsException: public std::exception {
	OutOfBoolsException(tracer::tag offset): static_offset(offset) {}
	tracer::tag static_offset;	
};
struct LoopBackException: public std::exception {
	LoopBackException(tracer::tag offset): static_offset(offset) {}
	tracer::tag static_offset;
};
struct MemoizationException: public std::exception {
	MemoizationException(tracer::tag offset, block::stmt_block::Ptr _parent, int32_t _child_id): static_offset(offset), parent(_parent), child_id(_child_id) {}
	tracer::tag static_offset;

	block::stmt_block::Ptr parent;
	int32_t child_id;
};
}

#endif

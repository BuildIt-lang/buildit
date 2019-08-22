#ifndef BLOCK_H
#define BLOCK_H
#include <memory>
#include <assert.h>
#include <iostream>
#include "blocks/block_visitor.h"
#include "util/tracer.h"

namespace builder {
class builder_context;

}
// Top level class definition for blocks
// Abstract class
namespace block {
class block;

template <typename T>
bool isa(std::shared_ptr<block> p) {
	std::shared_ptr<T> ret = std::dynamic_pointer_cast<T> (p);
	if (ret != nullptr) 
		return true;
	return false;
}
template <typename T>
std::shared_ptr<T> to(std::shared_ptr<block> p) {
	std::shared_ptr<T> ret = std::dynamic_pointer_cast<T> (p);
	assert(ret != nullptr);
	return ret;
}

class block: public std::enable_shared_from_this<block> {
public:
	typedef std::shared_ptr<block> Ptr;

	tracer::tag static_offset;

	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<block>());
	}
	template<typename T>
	std::shared_ptr<T> self() {
		return to<T>(shared_from_this());
	}	

	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)	
			return false;
		return true;
	}

};
}
#endif

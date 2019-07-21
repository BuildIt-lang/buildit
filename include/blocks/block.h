#ifndef BLOCK_H
#define BLOCK_H
#include <memory>
#include <assert.h>
#include <iostream>

namespace builder {
class builder_context;
}
// Top level class definition for blocks
// Abstract class
namespace block {
class block: public std::enable_shared_from_this<block> {
public:
	typedef std::shared_ptr<block> Ptr;

	int32_t static_offset;

	virtual void dump(std::ostream&, int);

};
template <typename T>
bool isa(block::Ptr p) {
	std::shared_ptr<T> ret = std::dynamic_pointer_cast<T> (p.get()->shared_from_this());

	if (ret != nullptr) 
		return true;
	return false;
}
template <typename T>
typename T::Ptr to(block::Ptr p) {
	typename T::Ptr ret = std::dynamic_pointer_cast<T> (p.get()->shared_from_this());
	assert(ret != nullptr);
	return ret;
}
}
#endif

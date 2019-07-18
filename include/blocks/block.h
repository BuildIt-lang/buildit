#ifndef BLOCK_H
#define BLOCK_H
#include <memory>
#include <assert.h>

// Top level class definition for blocks

// Abstract class
namespace block {
class block: public std::enable_shared_from_this<block> {
public:
	typedef std::shared_ptr<block> Ptr;
	virtual ~block() = default;
};
template <typename T>
bool isa(block::Ptr p) {
	std::shared_ptr<T> ret = std::dynamic_pointer_cast<T> (p);

	if (ret != nullptr) 
		return true;
	return false;
}
template <typename T>
typename T::Ptr to(block::Ptr p) {
	typename T::Ptr ret = std::dynamic_pointer_cast<T> (p);
	assert(ret != nullptr);
	return ret;
}
}
#endif

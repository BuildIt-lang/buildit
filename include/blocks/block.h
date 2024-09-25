#ifndef BLOCK_H
#define BLOCK_H
#include "blocks/block_visitor.h"
#include "util/tracer.h"
#include <assert.h>
#include <iostream>
#include <memory>
#include <unordered_map>

namespace builder {
class builder_context;
}
// Top level class definition for blocks
// Abstract class
namespace block {
class block;

template <typename T>
bool isa(std::shared_ptr<block> p) {
	std::shared_ptr<T> ret = std::dynamic_pointer_cast<T>(p);
	if (ret != nullptr)
		return true;
	return false;
}
template <typename T>
std::shared_ptr<T> to(std::shared_ptr<block> p) {
	std::shared_ptr<T> ret = std::dynamic_pointer_cast<T>(p);
	assert(ret != nullptr);
	return ret;
}

template <typename T>
std::shared_ptr<T> clone(std::shared_ptr<T> p) {
	if (!p) return nullptr;
	return to<T>(p->clone_impl());
}

template <typename T>
std::shared_ptr<T> clone_obj(T* t) {
	auto np = std::make_shared<T>();
	np->static_offset = t->static_offset;
	np->metadata_map = t->metadata_map;
	return np;
}

template <typename T>
class block_metadata_impl;

class block_metadata : public std::enable_shared_from_this<block_metadata> {
public:
	typedef std::shared_ptr<block_metadata> Ptr;
	virtual ~block_metadata() = default;

	template <typename T>
	bool isa(void) {
		if (std::dynamic_pointer_cast<block_metadata_impl<T>>(shared_from_this())) {
			return true;
		}
		return false;
	}

	template <typename T>
	std::shared_ptr<block_metadata_impl<T>> to(void) {
		std::shared_ptr<block_metadata_impl<T>> ret =
		    std::dynamic_pointer_cast<block_metadata_impl<T>>(shared_from_this());
		assert(ret != nullptr);
		return ret;
	}
};

template <typename T>
class block_metadata_impl : public block_metadata {
public:
	typedef std::shared_ptr<block_metadata_impl<T>> Ptr;
	T val;
	block_metadata_impl(T _val) : val(_val) {}
};



class block : public std::enable_shared_from_this<block> {
public:
	virtual ~block() = default;

	typedef std::shared_ptr<block> Ptr;

	tracer::tag static_offset;

	std::unordered_map<std::string, std::shared_ptr<block_metadata>> metadata_map;

	template <typename T>
	void setMetadata(std::string mdname, const T &val) {
		typename block_metadata_impl<T>::Ptr mdnode = std::make_shared<block_metadata_impl<T>>(val);
		metadata_map[mdname] = mdnode;
	}

	template <typename T>
	bool hasMetadata(std::string mdname) {
		if (metadata_map.find(mdname) == metadata_map.end())
			return false;
		typename block_metadata::Ptr mdnode = metadata_map[mdname];
		if (!mdnode->isa<T>())
			return false;
		return true;
	}

	template <typename T>
	T getMetadata(std::string mdname) {
		if (!hasMetadata<T>(mdname))
			assert(false && "No metadata with name specified");
		typename block_metadata::Ptr mdnode = metadata_map[mdname];
		return mdnode->to<T>()->val;
	}
	bool getBoolMetadata(std::string mdname) {
		return hasMetadata<bool>(mdname) && getMetadata<bool>(mdname);
	}

	virtual void dump(std::ostream &, int);
	virtual void accept(block_visitor *visitor) {
		visitor->visit(self<block>());
	}
	template <typename T>
	std::shared_ptr<T> self() {
		return to<T>(shared_from_this());
	}

	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		return true;
	}

	virtual block::Ptr clone_impl(void) {
		// abstract class always returns nullptr
		return nullptr;
	}
};
} // namespace block
#endif

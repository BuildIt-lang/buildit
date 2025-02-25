#ifndef BUILDER_GENERICS_H
#define BUILDER_GENERICS_H
#include "blocks/var.h"

namespace builder {

// TODO: Figure out if this needs to be wrapped into a generics sub namespace

// Generic placeholder class to be passed inside dyn_var
class generic {};


// An opaque handle over block::type
// this also allows us to overload operators and helper functions
class type {
public:
	block::type::Ptr enclosed_type;
	type(block::type::Ptr t): enclosed_type(t) {}

	bool operator==(const type& other) {
		return enclosed_type->is_same(other.enclosed_type);
	}

	bool operator!=(const type& other) {
		return !(*this == other);
	}
};


template <typename T>
type create_type(void) {
	return type(dyn_var<T>::create_block_type());
}

type type_of(const var& v);

type array_of(const type &t, int size = -1);
type remove_array(const type &t);
type remove_array(const type &t, int& size);
bool is_array(const type &t);

type pointer_of(const type &t);
bool is_pointer(const type &t);
type remove_pointer(const type &t);

}

#endif

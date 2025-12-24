#ifndef ND_VAR_H
#define ND_VAR_H

#include "builder/static_var.h"
#include "util/tracer.h"
#include "builder/exceptions.h"

namespace builder {

/* The new API for nd_var-able objects is simple
   We require the objects to follow data-flow lattice properties
   Each ND var wrapped T can have whatever state it wants, but it needs to derive from 
   nd_var_base (we don't need to have separate generators any more).
   nd_var_base objects will go across executions and will be stored
   in the invocation state. 

   The type T however needs to provide two functions : check(int) which checks if 
   a new value satisfies the current state of the object. merge(int) merges the new 
   value into the current state and also updates the generator.
   
*/

// Base class for nd_var wrappable objects
// We are not making any functions virtual but we declare them here to make sure 
// users declare them too
class nd_var_base {
protected: 
	nd_var_base() = default;
public:
	using value_type = void;

	bool check(int e) {
		assert(false && "Every derived type must define check");
		return false;
	}
	bool merge(int e) {
		assert(false && "Every deried type must define merge");
	}	
};

template <typename T, typename...Args>
std::shared_ptr<T> get_or_create_generator(tracer::tag req_tag, Args&&...args) {
	if (get_invocation_state()->nd_state_map.find(req_tag) == get_invocation_state()->nd_state_map.end()) {
		get_invocation_state()->nd_state_map[req_tag] = std::make_shared<T>(std::forward<Args>(args)...);
	}
	return std::static_pointer_cast<T>(get_invocation_state()->nd_state_map[req_tag]);
}

// A simple true at top boolean nd_var wrappable type
class true_top: public nd_var_base {
public:
	typedef enum {
		T = 1,
		F = 0,
	} value_t;

	value_t value;
	
	using value_type = value_t;
	
	true_top(value_t def): value(def) {}
	true_top(): value(F) {}

	bool check(value_t e) {
		if (value == T) return true;
		if (e == value) return true;
		return false;
	}
	void merge(value_t e) {
		if (e == F) return;
		value = e;
	}
};


template <typename T>
class nd_var {
	static_assert(std::is_base_of<nd_var_base, T>::value, "Types wrapped in nd_var must derive from nd_var_base");
	std::shared_ptr<T> val;	

public:
	template <typename...Args>
	nd_var(Args&&...args) {
		tracer::tag t = tracer::get_offset_in_function();
		val = get_or_create_generator<T>(t, std::forward<Args>(args)...);
	}

	// Allow access to wrapped value in case user wants to access object specific APIs	
	operator T& (void) {
		return *val;
	}
	operator const T& (void) const {
		return *val;
	}

	T* get(void) {
		return val.get();
	}
	const T* get(void) const {
		return val.get();
	}

	void require_val(typename T::value_type e) {
		// If the required value is compatible with the current state, 
		// return 
		if (val->check(e)) return;
		// Otherwise, merge update and throw
		val->merge(e);
		throw NonDeterministicFailureException();
	}

};



}
#endif

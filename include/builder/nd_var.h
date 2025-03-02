#ifndef ND_VAR_H
#define ND_VAR_H

#include "builder/builder.h"
#include "builder/static_var.h"
#include "util/tracer.h"
#include "builder/exceptions.h"

namespace builder {

template <typename T> 
struct nd_var {
};

struct nd_var_gen_base {
};

template <typename T>
struct nd_var_gen: public nd_var_gen_base {
};

template <>
struct nd_var_gen<bool>: public nd_var_gen_base {
				// false, // true	
	bool allowed_values[2] = {true, true};
};


template <typename T>
std::shared_ptr<nd_var_gen<T>> get_or_create_generator(tracer::tag req_tag) {
	std::string tag_name = req_tag.stringify();

	if (builder_context::current_builder_context->nd_state_map->find(tag_name) ==
		builder_context::current_builder_context->nd_state_map->end()) {
		(*(builder_context::current_builder_context->nd_state_map))[tag_name] = std::make_shared<nd_var_gen<T>>();
	}

	return std::static_pointer_cast<nd_var_gen<T>>(
		(*(builder_context::current_builder_context->nd_state_map))[tag_name]);
}

/* contstraints on bool types
Bool types are enumerable so it is suitable for nd_var
Since bool has only two values we don't need too many constraints. 
The only valid constraint is requires_equal. 

The lattice for bools also has just two points, 
the order can be specified by initial preferred value
*/

template <>
struct nd_var<bool> {

	static_var<int> current_value;
	tracer::tag current_tag;

	// Values can only be retrieved
	operator bool () const {
		return current_value;
	}

	void sample(bool default_value = false) {
		current_tag = get_offset_in_function();			
		auto generator = get_or_create_generator<bool>(current_tag);

		if (generator->allowed_values[default_value] == true)
			current_value = default_value;
		else if (generator->allowed_values[!default_value] == true)
			current_value = !default_value;
		else 
			assert(false && "nd bool has no possible values allowed");
	}

	nd_var(bool default_value = false) {
		sample(default_value);
	}
	
	void require_equal(bool value) {
		if (value == current_value) return;
		// We have sampled a wrong value, time to update 
		// the generator and reset

		auto generator = get_or_create_generator<bool>(current_tag);
		generator->allowed_values[current_value] = false;
	
		throw NonDeterministicFailureException();
	}
};


}
#endif

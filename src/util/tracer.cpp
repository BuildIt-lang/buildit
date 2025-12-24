#include "util/tracer.h"
#include "builder/builder_context.h"
#include "builder/static_var.h"
#include <string>

#ifdef TRACER_USE_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif

namespace builder {
extern void lambda_wrapper(std::function<void(void)>);
extern void lambda_wrapper_close(void);
} // namespace builder


namespace tracer {
#ifdef TRACER_USE_LIBUNWIND
tag get_offset_in_function(void) {
	unsigned long long function = (unsigned long long)(void *)builder::lambda_wrapper;
	unsigned long long function_end = (unsigned long long)(void *)builder::lambda_wrapper_close;
	unw_context_t context;
	unw_cursor_t cursor;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);

	tag new_tag;
	new_tag.dedup_id = 0;

	while (unw_step(&cursor)) {
		unw_word_t ip;
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		if ((unsigned long long)ip >= function && (unsigned long long)ip < function_end)
			break;
		new_tag.pointers.push_back((unsigned long long)ip);
	}

	// Now add snapshots of static vars
	for (auto tuple : builder::get_run_state()->deferred_static_var_tuples) {
		if (tuple == nullptr) {
			new_tag.static_var_snapshots.push_back(nullptr);
			continue;
		}
		new_tag.static_var_snapshots.push_back(tuple->snapshot());
		if (builder::get_builder_context()->enable_d2x) {
			new_tag.static_var_key_values.push_back({tuple->var_name, tuple->serialize()});
		}
	}
	for (auto tuple : builder::get_run_state()->static_var_tuples) {
		if (tuple == nullptr) {
			new_tag.static_var_snapshots.push_back(nullptr);
			continue;
		}
		new_tag.static_var_snapshots.push_back(tuple->snapshot());
		if (builder::get_builder_context()->enable_d2x) {
			new_tag.static_var_key_values.push_back({tuple->var_name, tuple->serialize()});
		}
	}

	// Finally add the live_dyn_var set
	new_tag.live_dyn_vars = builder::get_run_state()->live_dyn_vars;

	return new_tag;
}

#else
tag get_offset_in_function(void) {
	unsigned long long function = (unsigned long long)(void *)builder::lambda_wrapper;
	unsigned long long function_end = (unsigned long long)(void *)builder::lambda_wrapper_close;

	void *buffer[50];
	tag new_tag;
	new_tag.dedup_id = 0;

	// First add the RIP pointers
	int backtrace_size = backtrace(buffer, 50);
	for (int i = 0; i < backtrace_size; i++) {
		if ((unsigned long long)buffer[i] >= function && (unsigned long long)buffer[i] < function_end)
			break;
		new_tag.pointers.push_back((unsigned long long)buffer[i]);
	}

	// Now add snapshots of static vars

	for (auto tuple : builder::get_run_state()->deferred_static_var_tuples) {
		if (tuple == nullptr) {
			new_tag.static_var_snapshots.push_back(nullptr);
			continue;
		}
		new_tag.static_var_snapshots.push_back(tuple->snapshot());
		if (builder::get_builder_context()->enable_d2x) {
			new_tag.static_var_key_values.push_back({tuple->var_name, tuple->serialize()});
		}
	}
	for (auto tuple : builder::get_run_state()->static_var_tuples) {
		if (tuple == nullptr) {
			new_tag.static_var_snapshots.push_back(nullptr);
			continue;
		}
		new_tag.static_var_snapshots.push_back(tuple->snapshot());
		if (builder::get_builder_context()->enable_d2x) {
			new_tag.static_var_key_values.push_back({tuple->var_name, tuple->serialize()});
		}
	}

	// Finally add the live_dyn_var set
	new_tag.live_dyn_vars = builder::get_run_state()->live_dyn_vars;

	return new_tag;
}
#endif
static unsigned long long unique_tag_counter = 0;
tag get_unique_tag(void) {
	tag new_tag;
	new_tag.pointers.push_back(0);
	new_tag.pointers.push_back(unique_tag_counter);
	unique_tag_counter++;
	return new_tag;
}



bool tag::operator==(const tag &other) const {

	// Check the dedup id first since it is cheap
	if (dedup_id != other.dedup_id)
		return false;

	if (other.pointers.size() != pointers.size())
		return false;
	for (unsigned int i = 0; i < pointers.size(); i++)
		if (pointers[i] != other.pointers[i])
			return false;
	if (other.static_var_snapshots.size() != static_var_snapshots.size())
		return false;

	for (unsigned int i = 0; i < static_var_snapshots.size(); i++) {
		// If pointers to snapshots are equal, no need to compare
		if (static_var_snapshots[i] == other.static_var_snapshots[i]) 
			continue;
		// If one of the pointers is nullptr and the other isn't, return false
		if (static_var_snapshots[i] == nullptr)
			return false;

		// Now compare the actual snapshots
		if (!(static_var_snapshots[i]->operator == (other.static_var_snapshots[i])))
			return false;
		
	}

	// Finally compare the live_dyn_vars
	if (live_dyn_vars.size() != other.live_dyn_vars.size())
		return false;
	for (unsigned int i = 0; i < live_dyn_vars.size(); i++) 
		if (!(live_dyn_vars[i] == other.live_dyn_vars[i])) 
			return false;
	
	return true;
}

std::string tag::stringify(void) {
	if (cached_string != "")
		return cached_string;

	std::string output_string = "[";
	for (unsigned int i = 0; i < pointers.size(); i++) {
		char temp[128];
		sprintf(temp, "%llx", pointers[i]);
		output_string += temp;
		if (i != pointers.size() - 1)
			output_string += ", ";
	}
	output_string += "]:[";
	for (unsigned int i = 0; i < static_var_snapshots.size(); i++) {
		if (static_var_snapshots[i] == nullptr)
			output_string += "()";
		else 
			output_string += "(" + static_var_snapshots[i]->serialize() + ")";

		if (i != static_var_snapshots.size() - 1)
			output_string += ", ";
	}
	output_string += "]:[";

	for (unsigned int i = 0; i < live_dyn_vars.size(); i++) {
		output_string += std::to_string(live_dyn_vars[i]);
		if (i != live_dyn_vars.size() - 1) 
			output_string += ", ";
	}
	output_string += "]";

	// Finally add the dedup id
	output_string += "<" + std::to_string(dedup_id) + ">";

	cached_string = output_string;
	return output_string;
}

std::string tag::stringify_stat(void) {
	std::string output_string = "[";
	output_string += "]:[";
	for (unsigned int i = 0; i < static_var_snapshots.size(); i++) {
		if (static_var_snapshots[i] == nullptr)
			output_string += "()";
		else 
			output_string += "(" + static_var_snapshots[i]->serialize() + ")";

		if (i != static_var_snapshots.size() - 1)
			output_string += ", ";
	}
	output_string += "]:[]";

	return output_string;
}



} // namespace tracer

#ifndef TRACER_H
#define TRACER_H
#include <execinfo.h>
#include <string>
#include <vector>
#include <memory>
#include "util/hash_utils.h"

#include "builder/var_snapshots.h"

namespace builder {
class builder_context;
}

namespace tracer {

using tag_id = size_t;

class tag {
public:
	std::vector<unsigned long long> pointers;
	std::vector<std::shared_ptr<builder::static_var_snapshot_base>> static_var_snapshots;
	std::vector<std::pair<std::string, std::string>> static_var_key_values;
	std::vector<tag_id> live_dyn_vars;


	std::string cached_string;
	mutable size_t cached_hash = 0;

	bool operator==(const tag &other) const;
	bool operator!=(const tag &other) const {
		return !operator==(other);
	}
	bool is_empty(void) const {
		return pointers.size() == 0;
	}
	void clear(void) {
		pointers.clear();
		static_var_snapshots.clear();
		live_dyn_vars.clear();
	}

	// A function to create another tag
	// that just captures the LOC part
	// and ignores the static tags
	tag slice_loc(void) {
		tag new_tag;
		new_tag.pointers = pointers;
		return new_tag;
	}

	// Converts the static tag into a human 
	// readable string. Should be only used for 
	// debugging since it doesn't work for objects that
	// cannot be coverted to a string
	std::string stringify(void);

	std::string stringify_loc(void) {
		std::string output_string = "[";
		for (unsigned int i = 0; i < pointers.size(); i++) {
			char temp[128];
			sprintf(temp, "%llx", pointers[i]);
			output_string += temp;
			if (i != pointers.size() - 1)
				output_string += ", ";
		}
		output_string += "]:[";
		output_string += "]";

		return output_string;
	}
	std::string stringify_stat(void);

	size_t hash(void) const {
		if (cached_hash != 0) return cached_hash;

		// We will start by hashing the pointers
		size_t h = typeid(tag).hash_code();

		for (unsigned i = 0; i < pointers.size(); i++) {
			h = hash_combine(h, std::hash<unsigned long long>{}(pointers[1]));
		}
		// Now combine the hashes from each snapshot, snapshots take care of returning a fixed 
		// hash if the type itself isn't hashable - to avoid virtual disaptches, we will pre compute hashes 
		// in the base type

		for (unsigned i = 0; i < static_var_snapshots.size(); i++) {
			if (static_var_snapshots[i] == nullptr) continue;
			h = hash_combine(h, static_var_snapshots[i]->computed_hash);
		}

		// Finally combine the hash of the live_dyn_vars

		for (unsigned i = 0; i < live_dyn_vars.size(); i++) {
			h = hash_combine(h, (size_t) live_dyn_vars[i]);
		}

		cached_hash = h;
		return cached_hash;	
	}
};


tag get_unique_tag(void);

tag get_offset_in_function(void);

} // namespace tracer

// We will define a hash function for tag so it can be used to index into unordered_map/set

namespace std {

template <>
struct hash<tracer::tag> {
	size_t operator() (const tracer::tag& t) const {
		return t.hash();
	}
};

}

#endif

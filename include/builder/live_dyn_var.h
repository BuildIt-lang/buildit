#ifndef BUILDER_LIVE_DYN_VAR_H
#define BUILDER_LIVE_DYN_VAR_H

#include "util/tracer.h"
#include <unordered_set>

namespace builder {

class live_tag_set {
	std::unordered_set<tracer::tag> live_set;
	size_t computed_hash = 0;	
public:
	// Default empty set
	live_tag_set() {}

	live_tag_set(std::unordered_set<tracer::tag> &live_set): live_set(live_set) {
		// Now update the hash
		for (auto s: live_set) {
			computed_hash = tracer::hash_combine(computed_hash, s.hash());
		}
	}

	bool operator==(const live_tag_set& other) {
		if (this == &other) return true;
		if (computed_hash != other.computed_hash) return false;
		return live_set == other.live_set;
	}	
};


}

#endif

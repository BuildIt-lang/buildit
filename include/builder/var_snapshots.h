#ifndef BUILDER_VAR_SNAPSHOTS_H
#define BUILDER_VAR_SNAPSHOTS_H

#include "util/mtp_utils.h"

namespace builder {

// A base class for all static variable snapshots
// snapshots are shared_ptr allocated so that multiple tags
// can freely share snapshots, also allows freely caching snapshots
// to avoid excessive snapshot allocations

class static_var_snapshot_base: std::enable_shared_from_this<static_var_snapshot_base> {
public:
	typedef std::shared_ptr<static_var_snapshot_base> Ptr;
	virtual bool operator == (static_var_snapshot_base::Ptr other) = 0;
	virtual ~static_var_snapshot_base();
	virtual std::string serialize() = 0;

	// A precomputed hash separate from all implementations
	size_t computed_hash = 0;
};

template <typename T>
class static_var_snapshot: public static_var_snapshot_base {
public:
	typedef std::shared_ptr<static_var_snapshot<T>> Ptr;

	T snapshot;
	// T needs to be copy constructible
	static_var_snapshot (const T& s): snapshot(s) {
		computed_hash = tracer::hash_helper<T>::get_hash(s);
	}

	bool operator == (static_var_snapshot_base::Ptr _other) {
		if (computed_hash != _other->computed_hash) return false;
		Ptr other = std::dynamic_pointer_cast<static_var_snapshot<T>>(_other);
		if (!other) return false;
		// T needs to be comparable
		return (snapshot == other->snapshot);
	}	
	std::string serialize() {
		return utils::can_to_string<T>::get_string(snapshot);
	}
};

// Fixed sized arrays are handled differently
template <typename T, size_t size>
class static_var_snapshot<T[size]>: public static_var_snapshot_base {
public:
	typedef std::shared_ptr<static_var_snapshot<T[size]>> Ptr;
	
	// TODO: Can be optimized to use std::array, 
	// but initialization needs std::index_sequence

	std::vector<T> snapshot;
	static_var_snapshot (const T* s): snapshot(s, s + size) {
		if (snapshot.size() == 0) {
			computed_hash = typeid(T).hash_code();
		} else {
			computed_hash = tracer::hash_helper<T>::get_hash(snapshot[0]);
			for (unsigned i = 1; i < snapshot.size(); i++) {
				computed_hash = tracer::hash_combine(
					computed_hash, tracer::hash_helper<T>::get_hash(snapshot[i]));
			}
		}
	}

	bool operator == (static_var_snapshot_base::Ptr _other) {
		if (computed_hash != _other->computed_hash) return false;
		Ptr other = std::dynamic_pointer_cast<static_var_snapshot<T[size]>>(_other);
		if (!other) return false;
		// T needs to be comparable
		return (snapshot == other->snapshot);
	}	
	std::string serialize() {
		std::string output = "{";
		for (unsigned int i = 0; i < snapshot.size(); i++) {
			output += utils::can_to_string<T>::get_string(snapshot[i]);
			if (i != snapshot.size() - 1)
				output += ", ";
		}
		output += "}";
		return output;
	}
};

template <typename T>
class static_var_snapshot<T[]>: public static_var_snapshot_base {
public:
	typedef std::shared_ptr<static_var_snapshot<T[]>> Ptr;
	std::vector<T> snapshot;

	// Flexible array sizes would accept an extra parameter
	static_var_snapshot(const T* s, size_t size): snapshot(s, s + size) {
		if (snapshot.size() == 0) {
			computed_hash = typeid(T).hash_code();
		} else {
			computed_hash = tracer::hash_helper<T>::get_hash(snapshot[0]);
			for (unsigned i = 1; i < snapshot.size(); i++) {
				computed_hash = tracer::hash_combine(
					computed_hash, tracer::hash_helper<T>::get_hash(snapshot[i]));
			}
		}
	}

	bool operator == (static_var_snapshot_base::Ptr _other) {
		if (computed_hash != _other->computed_hash) return false;
		Ptr other = std::dynamic_pointer_cast<static_var_snapshot<T[]>>(_other);
		if (!other) return false;
		// T needs to be comparable
		return (snapshot == other->snapshot);
	}	
	std::string serialize() {
		std::string output = "{";
		for (unsigned int i = 0; i < snapshot.size(); i++) {
			output += utils::can_to_string<T>::get_string(snapshot[i]);
			if (i != snapshot.size() - 1)
				output += ", ";
		}
		output += "}";
		return output;
	}
};

// We don't need tracking tuples any more since static_vars themselves act 
// as tracking tuples

}

#endif 

#ifndef BUILDER_HASH_UTILS_H
#define BUILDER_HASH_UTILS_H

#include "util/mtp_utils.h"
#include <typeinfo>

namespace tracer {
// Just a hash_combine function
static inline size_t hash_combine(size_t h1, size_t h2) {
	// Boost's simple hash_combine. Works well for most cases
	return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
}

template <typename T, typename V=void>
struct hash_helper {
	// default case just returns hash of just the typeid
	static inline size_t get_hash(const T& _) {
		return typeid(T).hash_code();
	}
};

template <typename T>
struct hash_helper<T, typename utils::check_valid_type<decltype(std::hash<T>())>::type> {
	static inline size_t get_hash(const T& t) {
		return std::hash<T>{}(t);
	}
};

}

#endif

#ifndef BUILDER_MTP_UTILS_H
#define BUILDER_MTP_UTILS_H

#include <type_traits>
#include <string>

namespace utils {

template <typename T>
struct check_valid_type {
	typedef void type;
};

template <typename T, typename V=void>
struct has_operator_equal: std::false_type {};

template <typename T>
struct has_operator_equal<T, typename check_valid_type<decltype(std::declval<T>() == std::declval<T>())>::type>: std::true_type {};

template <typename T, typename V=void>
struct can_to_string {
	static std::string get_string(const T& t) {
		return "<no-str object>";
	}
};

template <typename T>
struct can_to_string<T, typename check_valid_type<decltype(std::to_string(std::declval<T>()))>::type> {
	static std::string get_string(const T& t) {
		return std::to_string(t);
	}
};

}

#endif

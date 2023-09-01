#ifndef TYPE_EXTRACTOR_H
#define TYPE_EXTRACTOR_H

#include "builder/forward_declarations.h"
#include <algorithm>

namespace builder {

struct custom_type_base;

// The main definition of the type extractor classes
template <typename T>
class type_extractor {
public:
	// This implementation is currenty only used
	// by custom types which are derived from custom_type_base
	static block::type::Ptr extract_type(void) {
		static_assert(std::is_base_of<custom_type_base, T>::value,
			      "Custom types should inherit from builder::custom_type_base");
		block::named_type::Ptr type = std::make_shared<block::named_type>();
		type->type_name = T::type_name;
		type->template_args = T::get_template_arg_types();
		return type;
	}
};

// Type specialization for basic C++ types
template <>
class type_extractor<short int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::SHORT_INT_TYPE;
		return type;
	}
};

template <>
class type_extractor<unsigned short int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::UNSIGNED_SHORT_INT_TYPE;
		return type;
	}
};

template <>
class type_extractor<int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::INT_TYPE;
		return type;
	}
};

template <>
class type_extractor<unsigned int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::UNSIGNED_INT_TYPE;
		return type;
	}
};
template <>
class type_extractor<long int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::LONG_INT_TYPE;
		return type;
	}
};

template <>
class type_extractor<unsigned long int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::UNSIGNED_LONG_INT_TYPE;
		return type;
	}
};
template <>
class type_extractor<long long int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::LONG_LONG_INT_TYPE;
		return type;
	}
};

template <>
class type_extractor<unsigned long long int> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::UNSIGNED_LONG_LONG_INT_TYPE;
		return type;
	}
};

template <>
class type_extractor<char> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::CHAR_TYPE;
		return type;
	}
};

template <>
class type_extractor<unsigned char> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::UNSIGNED_CHAR_TYPE;
		return type;
	}
};

template <>
class type_extractor<float> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::FLOAT_TYPE;
		return type;
	}
};

template <>
class type_extractor<double> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::DOUBLE_TYPE;
		return type;
	}
};

// Type specialization for pointer type
template <typename T>
class type_extractor<T *> {
public:
	static block::type::Ptr extract_type(void) {
		block::pointer_type::Ptr type = std::make_shared<block::pointer_type>();
		type->pointee_type = type_extractor<T>::extract_type();
		return type;
	}
};
// Type specialization for reference type
template <typename T>
class type_extractor<T &> {
public:
	static block::type::Ptr extract_type(void) {
		block::reference_type::Ptr type = std::make_shared<block::reference_type>();
		type->referenced_type = type_extractor<T>::extract_type();
		return type;
	}
};

template <>
class type_extractor<void> {
public:
	static block::type::Ptr extract_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::VOID_TYPE;
		return type;
	}
};

// Type specialization for array types
template <typename T, int x>
class type_extractor<T[x]> {
public:
	static block::type::Ptr extract_type(void) {
		block::array_type::Ptr type = std::make_shared<block::array_type>();
		type->element_type = type_extractor<T>::extract_type();
		type->size = x;
		return type;
	}
};

template <typename T>
class type_extractor<T[]> {
public:
	static block::type::Ptr extract_type(void) {
		block::array_type::Ptr type = block::to<block::array_type>(type_extractor<T[1]>::extract_type());
		type->size = -1;
		return type;
	}
};

// Type extractor for complete closure
template <typename T>
class type_extractor<dyn_var<T>> {
public:
	static block::type::Ptr extract_type(void) {
		block::builder_var_type::Ptr type = std::make_shared<block::builder_var_type>();
		type->builder_var_type_id = block::builder_var_type::DYN_VAR;
		type->closure_type = type_extractor<T>::extract_type();
		return type;
	}
};
template <typename T>
class type_extractor<static_var<T>> {
public:
	static block::type::Ptr extract_type(void) {
		block::builder_var_type::Ptr type = std::make_shared<block::builder_var_type>();
		type->builder_var_type_id = block::builder_var_type::STATIC_VAR;
		type->closure_type = type_extractor<T>::extract_type();
		return type;
	}
};

// Extracting function types
template <typename... args>
std::vector<block::type::Ptr> extract_type_vector_dyn(void);

template <typename T, typename... args>
std::vector<block::type::Ptr> extract_type_vector_helper_dyn(void) {
	std::vector<block::type::Ptr> rest = extract_type_vector_dyn<args...>();
	rest.push_back(type_extractor<T>::extract_type());
	return rest;
}

template <typename... args>
std::vector<block::type::Ptr> extract_type_vector_dyn(void) {
	return extract_type_vector_helper_dyn<args...>();
}

template <>
std::vector<block::type::Ptr> extract_type_vector_dyn<>(void);

template <typename r_type, typename... a_types>
class type_extractor<r_type(a_types...)> {
public:
	static block::type::Ptr extract_type(void) {
		block::function_type::Ptr type = std::make_shared<block::function_type>();
		type->return_type = type_extractor<r_type>::extract_type();
		type->arg_types = extract_type_vector_dyn<a_types...>();
		std::reverse(type->arg_types.begin(), type->arg_types.end());
		return type;
	}
};

template <const char *N, typename... Args>
struct name {};

template <typename... Args>
struct extract_type_from_args;

template <typename T1, typename... Args>
struct extract_type_from_args<T1, Args...> {
	static std::vector<block::type::Ptr> get_types() {
		auto a = extract_type_from_args<Args...>::get_types();
		a.insert(a.begin(), type_extractor<T1>::extract_type());
		return a;
	}
};

template <>
struct extract_type_from_args<> {
	static std::vector<block::type::Ptr> get_types() {
		return std::vector<block::type::Ptr>();
	}
};

template <const char *N, typename... Args>
class type_extractor<name<N, Args...>> {
public:
	static block::type::Ptr extract_type(void) {
		block::named_type::Ptr type = std::make_shared<block::named_type>();
		type->type_name = N;
		type->template_args = extract_type_from_args<Args...>::get_types();
		return type;
	}
};

} // namespace builder

#endif

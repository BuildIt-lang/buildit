#ifndef STATIC_VAR_H
#define STATIC_VAR_H

#include "builder/builder.h"
#include "builder/builder_context.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MAX_TRACKING_VAR_SIZE (128)

namespace builder {

template <typename T>
class static_var {
public:
	static_assert(std::is_same<T, short int>::value || std::is_same<T, unsigned short int>::value ||
			  std::is_same<T, int>::value || std::is_same<T, unsigned int>::value ||
			  std::is_same<T, long int>::value || std::is_same<T, unsigned long int>::value ||
			  std::is_same<T, long long int>::value || std::is_same<T, unsigned long long int>::value ||
			  std::is_same<T, char>::value || std::is_same<T, unsigned char>::value ||
			  std::is_same<T, float>::value || std::is_same<T, double>::value || std::is_pointer<T>::value,
		      "Currently builder::static_var is only supported for basic types\n");
	static_assert(sizeof(T) < MAX_TRACKING_VAR_SIZE, "Currently builder::static_var supports variables of max size "
							 "= " TOSTRING(MAX_TRACKING_VARIABLE_SIZE));
	T val;

	static_var(const static_var &other) : static_var((T)other) {}
	static_var &operator=(const static_var &other) {
		*this = (T)other;
		return *this;
	}

	template <typename OT>
	static_var(const static_var<OT> &other) : static_var((T)(OT)other) {}
	template <typename OT>
	static_var &operator=(const static_var<OT> &other) {
		*this = (T)(OT)other;
		return *this;
	}

	operator T &() {
		return val;
	}
	operator const T &() const {
		return val;
	}
	const T &operator=(const T &t) {
		val = t;
		return t;
	}
	static_var() {
		assert(builder_context::current_builder_context != nullptr);
		builder_context::current_builder_context->static_var_tuples.push_back(
		    tracking_tuple((unsigned char *)&val, sizeof(T)));
	}
	static_var(const T &v) {
		assert(builder_context::current_builder_context != nullptr);
		builder_context::current_builder_context->static_var_tuples.push_back(
		    tracking_tuple((unsigned char *)&val, sizeof(T)));
		val = v;
	}
	~static_var() {
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->static_var_tuples.size() > 0);
		assert(builder_context::current_builder_context->static_var_tuples.back().ptr == (unsigned char *)&val);
		builder_context::current_builder_context->static_var_tuples.pop_back();
	}
	operator builder() {
		return (builder)val;
	}
};

template <typename T>
class static_var<T[]> {
public:
	static_assert(std::is_same<T, short int>::value || std::is_same<T, unsigned short int>::value ||
			  std::is_same<T, int>::value || std::is_same<T, unsigned int>::value ||
			  std::is_same<T, long int>::value || std::is_same<T, unsigned long int>::value ||
			  std::is_same<T, long long int>::value || std::is_same<T, unsigned long long int>::value ||
			  std::is_same<T, char>::value || std::is_same<T, unsigned char>::value ||
			  std::is_same<T, float>::value || std::is_same<T, double>::value || std::is_pointer<T>::value,
		      "Currently builder::static_var arrays is only supported for basic types\n");
	static_assert(sizeof(T) < MAX_TRACKING_VAR_SIZE, "Currently builder::static_var supports variables of max size "
							 "= " TOSTRING(MAX_TRACKING_VARIABLE_SIZE));
	// Disable copy-assignment and initialization
	static_var(const static_var &x) = delete;
	static_var &operator=(const static_var &x) = delete;
	T *val = nullptr;

	T &operator[](size_t index) {
		return val[index];
	}
	const T &operator[](size_t index) const {
		return val[index];
	}
	static_var() {
		assert(builder_context::current_builder_context != nullptr);
		// This val _should_ not be used. But we will insert it to hold place
		// for this static var in the list of tuples, otherwise destructor order will be weird
		val = new T[1];

		builder_context::current_builder_context->static_var_tuples.push_back(
		    tracking_tuple((unsigned char *)val, 1));
	}
	static_var(const std::initializer_list<T> &list) {
		assert(builder_context::current_builder_context != nullptr);
		val = new T[list.size()];
		builder_context::current_builder_context->static_var_tuples.push_back(
		    tracking_tuple((unsigned char *)val, sizeof(T) * list.size()));
		for (int i = 0; i < list.size(); i++) {
			val[i] = list[i];
		}
	}
	void resize(size_t s) {
		T *new_ptr = new T[s];
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->static_var_tuples.size() > 0);
		for (size_t i = 0; i < builder_context::current_builder_context->static_var_tuples.size(); i++) {
			if (builder_context::current_builder_context->static_var_tuples[i].ptr ==
			    (unsigned char *)val) {
				builder_context::current_builder_context->static_var_tuples[i] =
				    tracking_tuple((unsigned char *)new_ptr, sizeof(T) * s);
				break;
			}
		}
		delete[] val;
		val = new_ptr;
	}
	~static_var() {
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->static_var_tuples.size() > 0);
		assert(builder_context::current_builder_context->static_var_tuples.back().ptr == (unsigned char *)val);
		builder_context::current_builder_context->static_var_tuples.pop_back();
		delete[] val;
	}
};
} // namespace builder
#endif

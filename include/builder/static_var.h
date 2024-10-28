#ifndef STATIC_VAR_H
#define STATIC_VAR_H

#include "builder/builder.h"
#include "builder/builder_context.h"
#include "util/var_finder.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MAX_TRACKING_VAR_SIZE (128)

namespace builder {

// Base class for all static vars
class static_var_base {
public:
	mutable std::string var_name;
	virtual std::string serialize() {
		return "";
	}
	virtual ~static_var_base() {}
};

template <typename T>
class static_var : static_var_base {
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
	bool is_deferred = false;

	mutable bool name_checked = false;
	void try_get_name() const {
		if (builder_context::current_builder_context->enable_d2x == false)
			return;
		if (var_name == "" && name_checked == false) {
			var_name = util::find_variable_name(const_cast<void *>(static_cast<const void *>(this)));
		}
	}

	static_var(const static_var &other) : static_var((T)other) {}
	static_var &operator=(const static_var &other) {
		try_get_name();
		name_checked = true;
		*this = (T)other;
		return *this;
	}

	template <typename OT>
	static_var(const static_var<OT> &other) : static_var((T)(OT)other) {}
	template <typename OT>
	static_var &operator=(const static_var<OT> &other) {
		try_get_name();
		name_checked = true;
		*this = (T)(OT)other;
		return *this;
	}

	operator T &() {
		try_get_name();
		name_checked = true;
		return val;
	}
	operator const T &() const {
		try_get_name();
		name_checked = true;
		return val;
	}
	const T &operator=(const T &t) {
		try_get_name();
		name_checked = true;
		val = t;
		return t;
	}
	static_var() {
		assert(builder_context::current_builder_context != nullptr);
		builder_context::current_builder_context->static_var_tuples.push_back(
		    tracking_tuple((unsigned char *)&val, sizeof(T), this));
		try_get_name();
	}
	static_var(const T &v) {
		assert(builder_context::current_builder_context != nullptr);
		builder_context::current_builder_context->static_var_tuples.push_back(
		    tracking_tuple((unsigned char *)&val, sizeof(T), this));
		val = v;
		try_get_name();
	}

	static_var(const defer_init &) {
		// Just like dynamic variables, no registration happens here
		is_deferred = true;
	}
	void deferred_init(void) {
		assert(builder_context::current_builder_context != nullptr);
		// Deferred static variables are kept separate because they are never untracked
		// in the destructor
		builder_context::current_builder_context->deferred_static_var_tuples.push_back(
		    tracking_tuple((unsigned char *)&val, sizeof(T), this));
		try_get_name();
	}

	~static_var() {
		if (is_deferred) {
			// Must be a deferred init object
			return;
		}

		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->static_var_tuples.size() > 0);

		/* Instead of assuming that the last variable is the static_var we are destroying,
		   we find the appropriate one and set its size to 0. Then we clean up all the trailing 0s 
		
		This allows out of order destruction while still maintaining good static tags */
		
		int index = -1;
		for (int i = builder_context::current_builder_context->static_var_tuples.size() - 1; i >= 0; i--) {
			if (builder_context::current_builder_context->static_var_tuples[i].ptr == (unsigned char*) &val) {
				index = i;
				builder_context::current_builder_context->static_var_tuples[i].size = 0;
				break;
			}
		}
		assert(index != -1 && "Static variable to destroy not valid");

		while (!builder_context::current_builder_context->static_var_tuples.empty() 
			&& builder_context::current_builder_context->static_var_tuples.back().size == 0) {
			builder_context::current_builder_context->static_var_tuples.pop_back();
		}
		
	}
	operator builder() {
		try_get_name();
		return (builder)val;
	}
	virtual std::string serialize() override {
		return std::to_string(val);
	}
};

template <typename T>
class static_var<T[]> : static_var_base {
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
		var_name = "ArrayVar";
		assert(builder_context::current_builder_context != nullptr);
		// This val _should_ not be used. But we will insert it to hold place
		// for this static var in the list of tuples, otherwise destructor order will be weird
		val = new T[1];

		builder_context::current_builder_context->static_var_tuples.push_back(
		    tracking_tuple((unsigned char *)val, 1, this));
	}
	static_var(const std::initializer_list<T> &list) {
		var_name = "ArrayVar";
		assert(builder_context::current_builder_context != nullptr);
		val = new T[list.size()];
		builder_context::current_builder_context->static_var_tuples.push_back(
		    tracking_tuple((unsigned char *)val, sizeof(T) * list.size(), this));
		for (int i = 0; i < list.size(); i++) {
			val[i] = list[i];
		}
	}
	void resize(size_t s) {
		var_name = "ArrayVar";
		T *new_ptr = new T[s];
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->static_var_tuples.size() > 0);
		for (size_t i = 0; i < builder_context::current_builder_context->static_var_tuples.size(); i++) {
			if (builder_context::current_builder_context->static_var_tuples[i].ptr ==
			    (unsigned char *)val) {
				builder_context::current_builder_context->static_var_tuples[i] =
				    tracking_tuple((unsigned char *)new_ptr, sizeof(T) * s, this);
				break;
			}
		}
		delete[] val;
		val = new_ptr;
	}
	~static_var() {
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->static_var_tuples.size() > 0);
		int index = -1;
		for (int i = builder_context::current_builder_context->static_var_tuples.size() - 1; i >= 0; i--) {
			if (builder_context::current_builder_context->static_var_tuples[i].ptr == (unsigned char*) val) {
				index = i;
				builder_context::current_builder_context->static_var_tuples[i].size = 0;
				break;
			}
		}
		assert(index != -1 && "Static variable to destroy not valid");

		while (!builder_context::current_builder_context->static_var_tuples.empty() 
			&& builder_context::current_builder_context->static_var_tuples.back().size == 0) {
			builder_context::current_builder_context->static_var_tuples.pop_back();
		}
		delete[] val;
	}
	virtual std::string serialize() override {
		return "<array>";
	}
};
} // namespace builder
#endif

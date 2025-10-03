#ifndef STATIC_VAR_H
#define STATIC_VAR_H

#include "builder/builder.h"
#include "builder/builder_context.h"
#include "util/var_finder.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

//#define MAX_TRACKING_VAR_SIZE (128)

namespace builder {

// Base class for all static vars
class static_var_base {
public:
	mutable std::string var_name;
	virtual std::string serialize() {
		return "";
	}

	virtual static_var_snapshot_base::Ptr snapshot() = 0;

	virtual ~static_var_base();
};



template <typename T>
class static_var : static_var_base {
public:

	static_assert(std::is_copy_constructible<T>::value && utils::has_operator_equal<T>::value, 
		"static_var can only be used with types that have == and are copy constructible");

	T val;
	bool is_deferred = false;

	mutable bool name_checked = false;

	void try_get_name() const {
		if (builder_context::current_builder_context->enable_d2x == false)
			return;
		if (var_name == "" && name_checked == false) {
			var_name = utils::find_variable_name(const_cast<void *>(static_cast<const void *>(this)));
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
		builder_context::current_builder_context->static_var_tuples.push_back(this);
		try_get_name();
	}
	static_var(const T &v) {
		assert(builder_context::current_builder_context != nullptr);
		builder_context::current_builder_context->static_var_tuples.push_back(this);
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
		// Even though the usual static vars can be destroyed out of order, these might create
		// unncessary bubbles in the queue
		// TODO: Consider merging these two
		builder_context::current_builder_context->deferred_static_var_tuples.push_back(this);
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
		   we find the appropriate one, remove it and replace it with nullptr. During tag creation
		   nullptrs are handled properly
		   
		   We then clear all the trailing nullptrs
		
		This allows out of order destruction while still maintaining good static tags */
		
		int index = -1;
		for (int i = builder_context::current_builder_context->static_var_tuples.size() - 1; i >= 0; i--) {
			if (builder_context::current_builder_context->static_var_tuples[i] == this) {
				index = i;
				builder_context::current_builder_context->static_var_tuples[i] = nullptr;
				break;
			}
		}
		assert(index != -1 && "Static variable to destroy not valid");

		while (!builder_context::current_builder_context->static_var_tuples.empty() 
			&& builder_context::current_builder_context->static_var_tuples.back() == nullptr) {
			builder_context::current_builder_context->static_var_tuples.pop_back();
		}
		
	}
	operator builder() {
		try_get_name();
		return (builder)val;
	}
	std::string serialize() override {
		return utils::can_to_string<T>::get_string(val);
	}
		
	static_var_snapshot_base::Ptr snapshot() override {
		return std::make_shared<static_var_snapshot<T>>(val);
	}
};

template <typename T>
class static_var<T[]> : static_var_base {
public:

	static_assert(std::is_copy_constructible<T>::value && utils::has_operator_equal<T>::value, 
		"static_var can only be used with types that have == and are copy constructible");

	// Disable copy-assignment and initialization
	static_var(const static_var &x) = delete;
	static_var &operator=(const static_var &x) = delete;

	T *val = nullptr;
	size_t actual_size = -1;

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
		actual_size = 1;
		builder_context::current_builder_context->static_var_tuples.push_back(this);
	}
	static_var(const std::initializer_list<T> &list) {
		var_name = "ArrayVar";
		assert(builder_context::current_builder_context != nullptr);
		val = new T[list.size()];
		actual_size = list.size();
		builder_context::current_builder_context->static_var_tuples.push_back(this);
		for (int i = 0; i < list.size(); i++) {
			val[i] = list[i];
		}
	}
	void resize(size_t s) {
		var_name = "ArrayVar";
		T *new_ptr = new T[s];
		delete[] val;
		val = new_ptr;
		actual_size = s;
		// tracking tuples dont' need to be changed anymore
	}
	~static_var() {
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->static_var_tuples.size() > 0);
		int index = -1;
		for (int i = builder_context::current_builder_context->static_var_tuples.size() - 1; i >= 0; i--) {
			if (builder_context::current_builder_context->static_var_tuples[i] == this) {
				index = i;
				builder_context::current_builder_context->static_var_tuples[i] = nullptr;
				break;
			}
		}
		assert(index != -1 && "Static variable to destroy not valid");

		while (!builder_context::current_builder_context->static_var_tuples.empty() 
			&& builder_context::current_builder_context->static_var_tuples.back() == nullptr) {
			builder_context::current_builder_context->static_var_tuples.pop_back();
		}
		delete[] val;
	}
	virtual std::string serialize() override {
		return "<array>";
	}
	static_var_snapshot_base::Ptr snapshot() override {
		return std::make_shared<static_var_snapshot<T[]>>(val, actual_size);
	}
};
} // namespace builder
#endif

#ifndef STATIC_VAR_H
#define STATIC_VAR_H

#include "builder/builder.h"
#include "builder/builder_context.h"
#include "util/var_finder.h"
#include <iostream>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MAX_TRACKING_VAR_SIZE (128)

namespace builder {

// Base class for all static variables
class static_var_base {
public:
	virtual std::string serialize();
	std::string var_name;
	virtual ~static_var_base();
};

template <typename T>
class static_var: public static_var_base {
public:
	static_assert(
		std::is_same<T, short int>::value || 
		std::is_same<T, unsigned short int>::value || 	
		std::is_same<T, int>::value || 
		std::is_same<T, unsigned int>::value || 
		std::is_same<T, long int>::value || 
		std::is_same<T, unsigned long int>::value || 
		std::is_same<T, long long int>::value || 
		std::is_same<T, unsigned long long int>::value || 
		std::is_same<T, char>::value || 
		std::is_same<T, unsigned char>::value || 	
		std::is_same<T, float>::value || 	
		std::is_same<T, double>::value || 	
		std::is_pointer<T>::value, "Currently builder::static_var is only supported for basic types\n");
	static_assert(sizeof(T) < MAX_TRACKING_VAR_SIZE, "Currently builder::static_var supports variables of max size "
							 "= " TOSTRING(MAX_TRACKING_VARIABLE_SIZE));
	T val;

	int name_checked = false;
	
	void check_at_access() {
		if (var_name == "" && name_checked == false) {
			// This is the last attempt at recovering the variable name
			std::string sep_save = util::member_separator;
			util::member_separator = ".";
			var_name = util::find_variable_name(this);
			util::member_separator = sep_save;
		}
		name_checked = true;
	}

	operator T &() { 
		check_at_access();
		return val; 
	}
	operator const T &() const { 
		return val; 
	}
	const T &operator=(const T &t) {	
		check_at_access();
		val = t;
		return t;
	}
	static_var() {
		assert(builder_context::current_builder_context != nullptr);
		builder_context::current_builder_context->static_var_tuples.push_back(tracking_tuple((unsigned char *)&val, sizeof(T), this));
		std::string sep_save = util::member_separator;
		util::member_separator = ".";
		var_name = util::find_variable_name(this);
		util::member_separator = sep_save;
	}
	static_var(const T &v) {
		assert(builder_context::current_builder_context != nullptr);
		builder_context::current_builder_context->static_var_tuples.push_back(tracking_tuple((unsigned char *)&val, sizeof(T), this));
		val = v;
		std::string sep_save = util::member_separator;
		util::member_separator = ".";
		var_name = util::find_variable_name(this);
		util::member_separator = sep_save;
	}
	static_var(const static_var& other) {
		assert(builder_context::current_builder_context != nullptr);
		builder_context::current_builder_context->static_var_tuples.push_back(tracking_tuple((unsigned char *)&val, sizeof(T), this));
		val = other.val;
		std::string sep_save = util::member_separator;
		util::member_separator = ".";
		var_name = util::find_variable_name(this);
		util::member_separator = sep_save;
	}
	~static_var() {
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->static_var_tuples.size() > 0);
		assert(builder_context::current_builder_context->static_var_tuples.back().ptr == (unsigned char *)&val);
		builder_context::current_builder_context->static_var_tuples.pop_back();
	}
	operator builder() { 	
		check_at_access();
		return (builder)val; 
	}

	virtual std::string serialize() override {
		// Assuming all static_vars can simply converted to string
		// This is currently true because of the static assert above
		return std::to_string(val);
	}
};

} // namespace builder
#endif

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
	operator T &() { return val; }
	operator const T &() const { return val; }
	const T &operator=(const T &t) {
		val = t;
		return t;
	}
	static_var() {
		assert(builder_context::current_builder_context != nullptr);
		builder_context::current_builder_context->static_var_tuples.push_back(tracking_tuple((unsigned char *)&val, sizeof(T)));
	}
	static_var(const T &v) {
		assert(builder_context::current_builder_context != nullptr);
		builder_context::current_builder_context->static_var_tuples.push_back(tracking_tuple((unsigned char *)&val, sizeof(T)));
		val = v;
	}
	~static_var() {
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->static_var_tuples.size() > 0);
		assert(builder_context::current_builder_context->static_var_tuples.back().ptr == (unsigned char *)&val);
		builder_context::current_builder_context->static_var_tuples.pop_back();
	}
	operator builder() { return (builder)val; }
};

} // namespace builder
#endif

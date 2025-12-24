#ifndef BUILDER_PROVIDERS_DYN_VAR_H
#define BUILDER_PROVIDERS_DYN_VAR_H
#include "util/mtp_utils.h"
#include "builder/block_type_extractor.h"
namespace builder {

template <typename T>
class dyn_var_impl;

template <typename T>
class dyn_var;


// A provider to compute return types for operator []
// Implicitly also disables the operator using SFINAE if the type
// isn't a pointer or if it doesn't have dereference type defined
template <typename T, typename V=void>
struct op_sq_bkt_ret_provider {};

template <typename T>
struct op_sq_bkt_ret_provider<T*, void> {
	using type = T;
};

template <typename T>
struct op_sq_bkt_ret_provider<T[], void>: public op_sq_bkt_ret_provider<T*> {
};
template <typename T, size_t s>
struct op_sq_bkt_ret_provider<T[s], void>: public op_sq_bkt_ret_provider<T*> {
};

template <typename T> 
struct op_sq_bkt_ret_provider<T, typename utils::check_valid_type<typename T::dereference_type>::type> {
	using type = typename T::dereference_type;
};

template <typename T>
using op_sq_bkt_ret_provider_t = typename op_sq_bkt_ret_provider<T>::type;

// A provider to compute return types for operator ()
// Implicitly also disables the operator using SFINAE if the 
// type isn't a function or function pointer type
// Argument types aren't enforced, to allow var args etc

template <typename T>
struct op_fcall_ret_provider {};

template <typename T>
struct op_fcall_ret_provider<T*>: public op_fcall_ret_provider<T> {};

template <typename RetType, typename...Args>
struct op_fcall_ret_provider<RetType(Args...)> {
	// Remove reference type from the return type in case the user uses auto
	using type = typename std::remove_reference<RetType>::type;
};

template <typename T>
using op_fcall_ret_provider_t = typename op_fcall_ret_provider<T>::type;

// A provider to inherit members from user defined types
// The dyn_var class will directly inherit from this type

extern bool user_defined_provider_track_members;

template <typename T>
struct user_defined_member_provider_begin {
	user_defined_member_provider_begin() {
		if (parents_stack == nullptr) {
			parents_stack = new std::vector<dyn_var_base*>();
		}
		parents_stack->push_back(static_cast<dyn_var_base*>(static_cast<dyn_var<T>*>(this)));
	}
};

struct user_defined_member_provider_end {
	user_defined_member_provider_end() {
		parents_stack->pop_back();
	}
};

template <typename T, typename TR=typename std::remove_reference<T>::type, typename V=void>
struct user_defined_member_provider {
}; // Default implementation is empty

template <typename T, typename TR>
struct user_defined_member_provider<T, TR, 
	typename std::enable_if<std::is_class<TR>::value && !is_dyn_var_type<TR>::value &&
		!is_static_var_type<TR>::value>::type>: 
	public user_defined_member_provider_begin<TR>, public TR, public user_defined_member_provider_end {		
};	


// A function that is implemented just for dyn_var<generic> that copies over RHS arguments
template <typename T, typename TO>
typename std::enable_if<!std::is_same<T, generic>::value>::type copy_types_provider(dyn_var_impl<T>& to, const TO& from) {
}

template <typename T, typename TO>
typename std::enable_if<std::is_same<T, generic>::value>::type copy_types_provider(dyn_var_impl<T>& to, const TO& from) {
	assert(to.var_mode == dyn_var_impl<T>::standalone_var);
	to.block_var->var_type = from.block_var->var_type;
}

// Not exaclty a helper but a base type user from inherit that provides
// template types to a type, required for stuff like user defined std::vector<T>
template <typename... Args>
struct custom_type {
	static std::vector<block::type::Ptr> get_template_arg_types() {
		return {type_extractor<Args>::extract_type()...};
	}
};


}

#endif

#ifndef BUILDER_FORWARD_DECLARATIONS_H
#define BUILDER_FORWARD_DECLARATIONS_H

#include "blocks/var.h"
#include <type_traits>
namespace builder {

class builder_root;

// The builder base class
class builder;

template <typename T>
using is_builder_type = typename std::is_same<builder, T>;

template <typename T>
using if_builder = typename std::enable_if<is_builder_type<T>::value, T>::type;

template <typename T>
class static_var;

// The var classes declaration
class var;

template <typename T>
class dyn_var_impl;

template <typename T>
class dyn_var;

template <typename T>
using is_dyn_var_type = std::is_base_of<var, T>;

template <typename T>
class type_extractor;

template <typename ClassName, typename RetType, class Enable, typename... AllArgs>
struct extract_signature;

template <typename T, typename... OtherArgs>
struct extract_signature_from_lambda;

// constructor helper to defer the initialization of dyn_var
// This allows declaring dyn_var outside the context, but initialize
// them later
struct defer_init {
	// No members
};


// Constructor helpers for dyn_var
struct as_global {
	std::string name;
	as_global(const std::string &n) : name(n) {}
};
// With name is just like as_global but can be used locally
struct with_name {
	std::string name;
	bool with_decl;
	with_name(const std::string &n, bool wd = false) : name(n), with_decl(wd) {}
};


} // namespace builder
#endif

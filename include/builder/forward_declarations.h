#ifndef BUILDER_FORWARD_DECLARATIONS_H
#define BUILDER_FORWARD_DECLARATIONS_H

#include <type_traits>
#include "blocks/var.h"
namespace builder {


class builder_root;

// The builder base class
class builder;

struct sentinel_member;


template<typename T>
using is_builder_type = typename std::is_base_of<builder_root, T>;

template<typename T>
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



// This class does nothing 
// Apart from just being used in the copy constructor to
// tell the constructor to no create without context
struct dyn_var_sentinel_type {

};

// This class is used for creating exact replicas of a variable
// One possible use if when initializing args for func_decls
class dyn_var_consume {
public:
	block::var::Ptr block_var = nullptr;
	dyn_var_consume(const var& a);
	dyn_var_consume(const dyn_var_consume&);
};
}
#endif


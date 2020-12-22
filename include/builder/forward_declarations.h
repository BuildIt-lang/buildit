#ifndef BUILDER_FORWARD_DECLARATIONS_H
#define BUILDER_FORWARD_DECLARATIONS_H

#include <type_traits>
namespace builder {

class builder_root;

// The builder base class
template <typename BT>
class builder_base;

struct sentinel_member;

using builder = builder_base<sentinel_member>;

template<typename T>
using is_builder_type = typename std::is_base_of<builder_root, T>;

template<typename T>
using if_builder = typename std::enable_if<is_builder_type<T>::value, T>::type;


template <typename T>
class static_var;

// The var classes declaration
class var;

template <typename T, typename MT, typename BT>
class dyn_var_base;

template <typename T>
using dyn_var = dyn_var_base<T, sentinel_member, builder>;

template <typename T>
using is_dyn_var_type = std::is_base_of<var, T>;



struct member_base;

template <typename T>
using is_member_type = std::is_base_of<member_base, T>;


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
}

#endif


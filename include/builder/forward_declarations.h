#ifndef BUILDER_FORWARD_DECLARATIONS_H
#define BUILDER_FORWARD_DECLARATIONS_H

namespace builder {

// The builder base class
template <typename BT>
class builder_base;

class builder;

// The static_var class

template <typename T>
class static_var;

// The var classes declaration
class var;


template <typename T, typename DVT, typename BT>
class dyn_var_base;

template <typename T>
class dyn_var;

template <typename T>
class type_extractor;

template<bool b> struct booltype {};

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


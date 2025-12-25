#ifndef BUILDER_FORWARD_DECLARATIONS_H
#define BUILDER_FORWARD_DECLARATIONS_H

#include "blocks/var.h"
#include "blocks/expr.h"
#include <type_traits>
namespace builder {

class static_var_base;

template <typename T>
class static_var;

template <typename T>
using is_static_var_type = std::is_base_of<static_var_base, T>;


// The var classes declaration
class dyn_var_base;

template <typename T>
class dyn_var;

template <typename T>
using is_dyn_var_type = std::is_base_of<dyn_var_base, T>;

template <typename T>
class type_extractor;

// constructor helper to defer the initialization of dyn_var
// This allows declaring dyn_var outside the context, but initialize
// them later
struct defer_init {
	// No members
};
// With name is just like as_global but can be used locally
struct with_name {
	std::string name;
	bool with_decl;
	with_name(const std::string &n, bool wd = false) : name(n), with_decl(wd) {}
};
// With block var constructor helper to create a dyn_var with an existing block var
// currently used by signature extract
struct with_block_var {
	block::var::Ptr var;
	bool with_decl;
	with_block_var(block::var::Ptr v, bool wd = false): var(v), with_decl(wd) {}
};

extern std::vector<dyn_var_base *> *parents_stack;
// Struct to initialize a dyn_var as member;
struct as_member {
	dyn_var_base *parent_var;
	std::string member_name;
	// This constructor is to be used if the user prefers to define a specialization for
	// dyn_var. In this case they do not inherit from custom_type
	as_member(dyn_var_base *p, std::string n) : parent_var(p), member_name(n){};
	as_member(std::string n) : parent_var(parents_stack->back()), member_name(n) {}
};

class type;
class generic;
// with type is defined in generics
struct with_type;

// Generator states for non-deterministic values
struct nd_var_gen_base;

// Helper functions called by users or otherwise
void annotate(std::string label);

} // namespace builder
#endif

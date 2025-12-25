#include "builder/dyn_var.h"
#include "blocks/var.h"
#include "builder/generics.h"
namespace builder {

type type_of(const dyn_var_base& v) {
	assert(v.block_var);
	// We want to avoid cloning variables that themselves don't have a type yet
	assert(v.block_var->var_type && "Cloning type from variable that doesn't have a type");
	return type(v.block_var->var_type);
}

type array_of(const type &t, int size) {
	block::array_type::Ptr at = std::make_shared<block::array_type>();
	at->element_type = t.enclosed_type;
	at->size = size;
	return type(at);
}

bool is_array(const type &t) {
	return block::isa<block::array_type>(t.enclosed_type);
}
type remove_array(const type &t) {
	return type(block::to<block::array_type>(t.enclosed_type)->element_type);
}
type remove_array(const type &t, int &size) {
	size = block::to<block::array_type>(t.enclosed_type)->size;
	return type(block::to<block::array_type>(t.enclosed_type)->element_type);
}

type pointer_of(const type &t) {
	block::pointer_type::Ptr pt = std::make_shared<block::pointer_type>();
	pt->pointee_type = t.enclosed_type;
	return type(pt);
}
bool is_pointer(const type& t) {
	return block::isa<block::pointer_type>(t.enclosed_type);
}
type remove_pointer(const type &t) {
	return type(block::to<block::pointer_type>(t.enclosed_type)->pointee_type);
}


}

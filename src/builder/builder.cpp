#include "builder/builder.h"
#include "builder/builder_context.h"
#include "util/tracer.h"
#include "builder/dyn_var.h"

namespace builder {

builder var::operator!() { return !(builder) * this; }



var::operator bool() { return (bool)(builder) * this; }

template <>
std::vector<block::type::Ptr> extract_type_vector_dyn<>(void) {
	std::vector<block::type::Ptr> empty_vector;
	return empty_vector;
}
template <>
std::vector<block::expr::Ptr> extract_call_arguments<>(void) {
	std::vector<block::expr::Ptr> empty_vector;
	return empty_vector;
}
template <>
std::vector<block::expr::Ptr> extract_call_arguments_helper<>(void) {
	std::vector<block::expr::Ptr> empty_vector;
	return empty_vector;
}


} // namespace builder

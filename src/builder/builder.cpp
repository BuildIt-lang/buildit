#include "builder/builder.h"
#include "builder/builder_context.h"
#include "util/tracer.h"
#include "builder/dyn_var.h"

namespace builder {





template <>
std::vector<block::type::Ptr> extract_type_vector_dyn<>(void) {
	std::vector<block::type::Ptr> empty_vector;
	return empty_vector;
}

block::expr::Ptr member_base::get_parent() const {
	return nullptr;
}
dyn_var_consume::dyn_var_consume(const var& a) {
	block_var = a.block_var;
}
dyn_var_consume::dyn_var_consume(const dyn_var_consume& a) {
	block_var = a.block_var;
}

} // namespace builder

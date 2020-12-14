#include "builder/builder.h"
#include "builder/builder_context.h"
#include "util/tracer.h"

namespace builder {

/*
builder var::operator&&(const builder &a) { return (builder) * this && a; }
builder var::operator||(const builder &a) { return (builder) * this || a; }
builder var::operator+(const builder &a) { return (builder) * this + a; }
builder var::operator-(const builder &a) { return (builder) * this - a; }
builder var::operator*(const builder &a) { return (builder) * this * a; }
builder var::operator/(const builder &a) { return (builder) * this / a; }
builder var::operator<(const builder &a) { return (builder) * this < a; }
builder var::operator>(const builder &a) { return (builder) * this > a; }
builder var::operator<=(const builder &a) { return (builder) * this <= a; }
builder var::operator>=(const builder &a) { return (builder) * this >= a; }
builder var::operator==(const builder &a) { return (builder) * this == a; }
builder var::operator!=(const builder &a) { return (builder) * this != a; }
builder var::operator%(const builder &a) { return (builder) * this % a; }
*/
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

void create_return_stmt(const builder a) {
	assert(builder_context::current_builder_context != nullptr);
	builder_context::current_builder_context->remove_node_from_sequence(
	    a.block_expr);
	assert(builder_context::current_builder_context->current_block_stmt !=
	       nullptr);
	builder_context::current_builder_context->commit_uncommitted();

	block::return_stmt::Ptr ret_stmt =
	    std::make_shared<block::return_stmt>();
	ret_stmt->static_offset = a.block_expr->static_offset;
	ret_stmt->return_val = a.block_expr;
	builder_context::current_builder_context->add_stmt_to_current_block(
	    ret_stmt);
}

} // namespace builder

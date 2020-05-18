#include "builder/builder.h"
#include "util/tracer.h"
#include "builder/builder_context.h"

namespace builder {
builder builder::sentinel_builder;
/*
var::operator builder () const {
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return builder::sentinel_builder;
	assert(block_var != nullptr);
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
		
	block::var_expr::Ptr var_expr = std::make_shared<block::var_expr>();
	var_expr->static_offset = offset;
	
	var_expr->var1 = block_var;
	builder_context::current_builder_context->add_node_to_sequence(var_expr);
	
	builder ret_builder;
	ret_builder.block_expr = var_expr;
	return ret_builder;
}
*/

builder::builder(const var& a) {
	assert(builder_context::current_builder_context != nullptr);
	block_expr = nullptr;
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return;
	assert(a.block_var != nullptr);
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
		
	block::var_expr::Ptr var_expr = std::make_shared<block::var_expr>();
	var_expr->static_offset = offset;
	
	var_expr->var1 = a.block_var;
	builder_context::current_builder_context->add_node_to_sequence(var_expr);
	
	block_expr = var_expr;
}



builder::builder (const int &a) {	
	assert(builder_context::current_builder_context != nullptr);
	block_expr = nullptr;
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return;
	block::int_const::Ptr int_const = std::make_shared<block::int_const>();
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	//assert(offset != -1);
	int_const->static_offset = offset;
	int_const->value = a; 
	builder_context::current_builder_context->add_node_to_sequence(int_const);	
	
	block_expr = int_const;
	
}
builder::builder (const double &a) {	
	assert(builder_context::current_builder_context != nullptr);
	block_expr = nullptr;
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return;
	block::double_const::Ptr double_const = std::make_shared<block::double_const>();
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	double_const->static_offset = offset;
	double_const->value = a; 
	builder_context::current_builder_context->add_node_to_sequence(double_const);	
	
	block_expr = double_const;
	
}
builder::builder (const float &a) {	
	assert(builder_context::current_builder_context != nullptr);
	block_expr = nullptr;
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return;
	block::float_const::Ptr float_const = std::make_shared<block::float_const>();
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	float_const->static_offset = offset;
	float_const->value = a; 
	builder_context::current_builder_context->add_node_to_sequence(float_const);	
	
	block_expr = float_const;
	
}
template <typename T>
builder builder::builder_unary_op() const {
	assert(builder_context::current_builder_context != nullptr);
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return builder::sentinel_builder;
	
	builder_context::current_builder_context->remove_node_from_sequence(block_expr);
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	//assert(offset != -1);
	
	typename T::Ptr expr = std::make_shared<T>();
	expr->static_offset = offset;
	

	expr->expr1 = block_expr;

	builder_context::current_builder_context->add_node_to_sequence(expr);	
	
	builder ret_builder;
	ret_builder.block_expr = expr;
	return ret_builder;	
}
template <typename T>
builder builder::builder_binary_op(const builder &a) const {
	assert(builder_context::current_builder_context != nullptr);
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return builder::sentinel_builder;
	
	builder_context::current_builder_context->remove_node_from_sequence(block_expr);
	builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);

	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	//assert(offset != -1);
	
	typename T::Ptr expr = std::make_shared<T>();
	expr->static_offset = offset;
	

	expr->expr1 = block_expr;
	expr->expr2 = a.block_expr;

	builder_context::current_builder_context->add_node_to_sequence(expr);	
	
	builder ret_builder;
	ret_builder.block_expr = expr;
	return ret_builder;	
}

builder operator && (const builder &a, const builder& b) {
	return a.builder_binary_op<block::and_expr>(b);
}
builder operator && (const builder &a, const bool& b) {	
	return a.builder_binary_op<block::and_expr>((builder)b);
}

builder operator || (const builder &a, const builder &b) {
	return a.builder_binary_op<block::or_expr>(b);
}
builder operator || (const builder &a, const bool& b) {
	return a.builder_binary_op<block::or_expr>((builder)b);
}


builder operator + (const builder &a, const builder &b) {
	return a.builder_binary_op<block::plus_expr>(b);
}

builder operator - (const builder &a, const builder &b) {
	return a.builder_binary_op<block::minus_expr>(b);
}

builder operator * (const builder &a, const builder &b) {
	return a.builder_binary_op<block::mul_expr>(b);
}

builder operator / (const builder &a, const builder &b) {
	return a.builder_binary_op<block::div_expr>(b);
}

builder operator < (const builder &a, const builder &b) {
	return a.builder_binary_op<block::lt_expr>(b);
}

builder operator > (const builder &a, const builder &b) {
	return a.builder_binary_op<block::gt_expr>(b);
}

builder operator <= (const builder &a, const builder &b) {
	return a.builder_binary_op<block::lte_expr>(b);
}

builder operator >= (const builder &a, const builder &b) {
	return a.builder_binary_op<block::gte_expr>(b);
}

builder operator == (const builder &a, const builder &b) {
	return a.builder_binary_op<block::equals_expr>(b);
}

builder operator != (const builder &a, const builder &b) {
	return a.builder_binary_op<block::ne_expr>(b);
}

builder operator % (const builder &a, const builder &b) {
	return a.builder_binary_op<block::mod_expr>(b);
}

builder var::operator && (const builder& a) {
	return (builder)*this && a;
}

builder var::operator || (const builder& a) {
	return (builder)*this || a;
}
builder var::operator + (const builder& a) {
	return (builder)*this + a;
}
builder var::operator - (const builder& a) {
	return (builder)*this - a;
}
builder var::operator * (const builder& a) {
	return (builder)*this * a;
}
builder var::operator / (const builder& a) {
	return (builder)*this / a;
}
builder var::operator < (const builder& a) {
	return (builder)*this < a;
}
builder var::operator > (const builder& a) {
	return (builder)*this > a;
}
builder var::operator <= (const builder& a) {
	return (builder)*this <= a;
}
builder var::operator >= (const builder& a) {
	return (builder)*this >= a;
}
builder var::operator == (const builder& a) {
	return (builder)*this == a;
}
builder var::operator != (const builder& a) {
	return (builder)*this != a;
}
builder var::operator % (const builder& a) {
	return (builder)*this % a;
}
builder var::operator ! () {
	return !(builder)*this;
}

builder builder::operator [] (const builder &a) {
	assert(builder_context::current_builder_context != nullptr);
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return builder::sentinel_builder;
	
	builder_context::current_builder_context->remove_node_from_sequence(block_expr);
	builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);

	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	//assert(offset != -1);
	
	block::sq_bkt_expr::Ptr expr = std::make_shared<block::sq_bkt_expr>();
	expr->static_offset = offset;
	

	expr->var_expr = block_expr;
	expr->index = a.block_expr;

	builder_context::current_builder_context->add_node_to_sequence(expr);	
	
	builder ret_builder;
	ret_builder.block_expr = expr;
	return ret_builder;	
}
builder var::operator [] (const builder &a) {
	return ((builder)(*this))  [a];
}

builder operator ! (const builder &a) {
	return a.builder_unary_op<block::not_expr>();
}

builder builder::operator = (const builder &a) {
	assert(builder_context::current_builder_context != nullptr);
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return builder::sentinel_builder;
	
	builder_context::current_builder_context->remove_node_from_sequence(block_expr);
	builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	//assert(offset != -1);
	
	block::assign_expr::Ptr expr = std::make_shared<block::assign_expr>();
	expr->static_offset = offset;
	

	expr->var1 = block_expr;
	expr->expr1 = a.block_expr;

	builder_context::current_builder_context->add_node_to_sequence(expr);	
	
	builder ret_builder;
	ret_builder.block_expr = expr;
	return ret_builder;	
}
builder var::operator = (const builder &a) {
	return (builder)*this = a;
}

builder::operator bool() {
	builder_context::current_builder_context->commit_uncommitted();
	return get_next_bool_from_context(builder_context::current_builder_context, block_expr);
}

var::operator bool() {
	return (bool)(builder)*this;
}



template <>
std::vector<block::type::Ptr> extract_type_vector_dyn<> (void) {
	std::vector<block::type::Ptr> empty_vector;
	return empty_vector;
}

template <>
std::vector <block::expr::Ptr> extract_call_arguments<> (void) {
	std::vector<block::expr::Ptr> empty_vector;
	return empty_vector;
}
void create_return_stmt(const builder a) {
	assert(builder_context::current_builder_context != nullptr);
	builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);
	assert(builder_context::current_builder_context->current_block_stmt != nullptr);
	
	block::return_stmt::Ptr ret_stmt = std::make_shared<block::return_stmt>();
	ret_stmt->static_offset = a.block_expr->static_offset;
	ret_stmt->return_val = a.block_expr;
	builder_context::current_builder_context->add_stmt_to_current_block(ret_stmt);
}

}

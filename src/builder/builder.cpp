#include "builder/builder.h"
#include "util/tracer.h"
#include "builder/builder_context.h"

namespace builder {
var::operator builder () const {
	assert(block_var != nullptr);
	int32_t offset = get_offset_in_function(builder_context::current_builder_context->current_function);
		
	block::var_expr::Ptr var_expr = std::make_shared<block::var_expr>();
	var_expr->static_offset = offset;
	
	var_expr->var1 = block_var;
	builder_context::current_builder_context->add_node_to_sequence(var_expr);
	
	builder ret_block;
	ret_block.block_expr = var_expr;
	
	return ret_block;
}
void int_var::create_int_var(void) {
	assert(builder_context::current_builder_context != nullptr);
	assert(builder_context::current_builder_context->current_block_stmt != nullptr);
	
	builder_context::current_builder_context->commit_uncommitted();
	
	block::int_var::Ptr int_var = std::make_shared<block::int_var>();	
		
	block_var = int_var;
	
	int32_t offset = get_offset_in_function(builder_context::current_builder_context->current_function);
		
	block::decl_stmt::Ptr decl_stmt = std::make_shared<block::decl_stmt>();
	decl_stmt->static_offset = offset;
	builder_context::current_builder_context->visited_offsets.insert(offset);
	
	decl_stmt->decl_var = int_var;
	decl_stmt->init_expr = nullptr;
	block_decl_stmt = decl_stmt;
	
	builder_context::current_builder_context->current_block_stmt->stmts.push_back(decl_stmt);
	int_var->static_offset = offset;
	
}	
int_var::int_var() {
	create_int_var();
}		
int_var::int_var(const int_var& a): int_var((builder)a) {
}
int_var::int_var(const builder& a) {
	builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);
	create_int_var();
	block_decl_stmt->init_expr = a.block_expr;	
}
int_var::int_var(const int a): int_var((builder)a) {
}

builder::builder (const int &a) {	
	assert(builder_context::current_builder_context != nullptr);
	block::int_const::Ptr int_const = std::make_shared<block::int_const>();
	int32_t offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	assert(offset != -1);
	int_const->static_offset = offset;
	int_const->value = a; 
	builder_context::current_builder_context->add_node_to_sequence(int_const);	
	
	block_expr = int_const;
}
template <typename T>
builder builder::builder_unary_op() {
	assert(builder_context::current_builder_context != nullptr);
	
	builder_context::current_builder_context->remove_node_from_sequence(block_expr);
	int32_t offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	assert(offset != -1);
	
	typename T::Ptr expr = std::make_shared<T>();
	expr->static_offset = offset;
	

	expr->expr1 = block_expr;

	builder_context::current_builder_context->add_node_to_sequence(expr);	
	
	builder ret_builder;
	ret_builder.block_expr = expr;
	return ret_builder;	
}
template <typename T>
builder builder::builder_binary_op(const builder &a) {
	assert(builder_context::current_builder_context != nullptr);
	
	builder_context::current_builder_context->remove_node_from_sequence(block_expr);
	builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);

	int32_t offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	assert(offset != -1);
	
	typename T::Ptr expr = std::make_shared<T>();
	expr->static_offset = offset;
	

	expr->expr1 = block_expr;
	expr->expr2 = a.block_expr;

	builder_context::current_builder_context->add_node_to_sequence(expr);	
	
	builder ret_builder;
	ret_builder.block_expr = expr;
	return ret_builder;	
}

builder builder::operator && (const builder &a) {
	return builder_binary_op<block::and_expr>(a);
}
builder var::operator && (const builder &a) {
	return this->operator builder() && a;
}
builder operator && (const int &a, const builder &b) {
	return (builder)a && b;
}

builder builder::operator || (const builder &a) {
	return builder_binary_op<block::or_expr>(a);
}
builder var::operator || (const builder &a) {
	return this->operator builder() || a;
}
builder operator || (const int &a, const builder &b) {
	return (builder)a || b;
}

builder builder::operator + (const builder &a) {
	return builder_binary_op<block::plus_expr>(a);
}
builder var::operator + (const builder &a) {
	return this->operator builder() + a;
}
builder operator + (const int &a, const builder &b) {
	return (builder)a + b;
}

builder builder::operator - (const builder &a) {
	return builder_binary_op<block::minus_expr>(a);
}
builder var::operator - (const builder &a) {
	return this->operator builder() - a;
}
builder operator - (const int &a, const builder &b) {
	return (builder)a - b;
}

builder builder::operator * (const builder &a) {
	return builder_binary_op<block::mul_expr>(a);
}
builder var::operator * (const builder &a) {
	return this->operator builder() * a;
}
builder operator * (const int &a, const builder &b) {
	return (builder)a * b;
}

builder builder::operator / (const builder &a) {
	return builder_binary_op<block::div_expr>(a);
}
builder var::operator / (const builder &a) {
	return this->operator builder() / a;
}
builder operator / (const int &a, const builder &b) {
	return (builder)a / b;
}

builder builder::operator ! () {
	return builder_unary_op<block::not_expr>();
}
builder var::operator! () {
	return !this->operator builder();
}
builder var::operator = (const builder &a) {
	assert(builder_context::current_builder_context != nullptr);
	
	builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);
	int32_t offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	assert(offset != -1);
	
	block::assign_expr::Ptr expr = std::make_shared<block::assign_expr>();
	expr->static_offset = offset;
	

	expr->var1 = block_var;
	expr->expr1 = a.block_expr;

	builder_context::current_builder_context->add_node_to_sequence(expr);	
	
	builder ret_builder;
	ret_builder.block_expr = expr;
	return ret_builder;	
}

builder::operator bool() {
	return get_next_bool_from_context(builder_context::current_builder_context, block_expr);
}
var::operator bool() {
	return (bool)this->operator builder();
}
}

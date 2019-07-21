#include "builder/builder.h"
#include "util/tracer.h"
#include "builder/builder_context.h"

namespace builder {
var::operator builder () const {
	assert(block_var != nullptr);
	int32_t offset = get_offset_in_function(context->current_function);
		
	block::var_expr::Ptr var_expr = std::make_shared<block::var_expr>();
	var_expr->static_offset = offset;
	
	var_expr->var1 = block_var;
	context->add_node_to_sequence(var_expr);
	
	builder ret_block;
	ret_block.context = context;	
	ret_block.block_expr = var_expr;
	
	return ret_block;
}
	
int_var::int_var(builder_context *context_) {
	assert(context_ != nullptr);
	assert(context_->current_block_stmt != nullptr);
	context = context_;
	context->commit_uncommitted();
	
	block::int_var::Ptr int_var = std::make_shared<block::int_var>();	
		
	block_var = int_var;
	
	int32_t offset = get_offset_in_function(context->current_function);
			
	block::decl_stmt::Ptr decl_stmt = std::make_shared<block::decl_stmt>();
	decl_stmt->static_offset = offset;
	
	decl_stmt->decl_var = int_var;
	decl_stmt->init_expr = nullptr;
	
	context->current_block_stmt->stmts.push_back(decl_stmt);


	var_name = std::string("var") + std::to_string(context->var_name_counter);
	context->var_name_counter++;
	
	block_var->var_name = var_name;
}		


builder builder::operator && (const builder &a) {
	assert(context != nullptr);
	assert(context == a.context);
	
	int32_t offset = get_offset_in_function(context->current_function);
	assert(offset != -1);
	
	block::and_expr::Ptr expr = std::make_shared<block::and_expr>();
	expr->static_offset = offset;
	
	context->remove_node_from_sequence(block_expr);
	context->remove_node_from_sequence(a.block_expr);

	expr->expr1 = block_expr;
	expr->expr2 = a.block_expr;

	context->add_node_to_sequence(expr);	
	
	builder ret_builder;
	ret_builder.context = context;
	ret_builder.block_expr = expr;
	return ret_builder;	
}
builder var::operator && (const builder &a) {
	return (builder)(*this) && a;
}
builder::operator bool() {
	return get_next_bool_from_context(context, block_expr);
}
}

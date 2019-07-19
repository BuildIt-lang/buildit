#include "builder/builder.h"

namespace builder {
var::operator block::expr::Ptr () const {
	assert(block_var != nullptr);
	
	block::var_expr::Ptr var_expr = std::make_shared<block::var_expr>();
	var_expr->context = context;
	
	var_expr->var1 = block_var;
	context->add_node_to_sequence(var_expr);
	
	return var_expr;	
}
	
int_var::int_var(builder_context *context_) {
	assert(context_ != nullptr);
	context = context_;
	context->commit_uncommitted();
	
	block::int_var::Ptr int_var = std::make_shared<block::int_var>();
	int_var->context = context;
		
	block_var = int_var;
	
			
	block::decl_stmt::Ptr decl_stmt = std::make_shared<block::decl_stmt>();
	decl_stmt->context = context;
	
	decl_stmt->decl_var = int_var;
	decl_stmt->init_expr = nullptr;
	
	context->current_block_stmt->stmts.push_back(decl_stmt);
}		

block::expr::Ptr operator && (const var& a, const block::expr::Ptr& b) {	
	return (block::expr::Ptr)a && b;
}
}

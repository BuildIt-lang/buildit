#include "builder/builder_context.h"
#include "builder/exceptions.h"
#include "util/tracer.h"
#include <algorithm>
namespace builder {
builder_context::builder_context() {
	current_block_stmt = nullptr;
	ast = nullptr;
}
void builder_context::commit_uncommitted(void) {
	for (auto block_ptr: uncommitted_sequence) {
		block::expr_stmt::Ptr s = std::make_shared<block::expr_stmt>();
		assert(block::isa<block::expr>(block_ptr));
		s->context = block_ptr->context;
		s->static_offset = block_ptr->static_offset;
		s->expr1 = block::to<block::expr>(block_ptr);
		assert(current_block_stmt != nullptr);
		current_block_stmt->stmts.push_back(s);			
	}	
	uncommitted_sequence.clear();

}
void builder_context::remove_node_from_sequence(block::expr::Ptr e) {
	uncommitted_sequence.remove(e);
}
void builder_context::add_node_to_sequence(block::expr::Ptr e) {
	uncommitted_sequence.push_back(e);
}
block::stmt::Ptr builder_context::extract_ast(void) {
	commit_uncommitted();
	return ast;
}

bool get_next_bool_from_builder_context(builder_context *context, block::expr::Ptr expr) {	
	if (context->bool_vector.size() == 0) {
		int32_t offset = expr->static_offset; 
		throw OutOfBoolsException(offset);
	}
	bool ret_val = context->bool_vector.back();
	context->bool_vector.pop_back();
	return ret_val; 
}


static void trim_ast_at_offset(block::stmt::Ptr ast, int32_t offset) {
	block::stmt_block::Ptr top_level_block = block::to<block::stmt_block>(ast);
	std::vector<block::stmt::Ptr> &stmts = top_level_block->stmts;
	auto it = stmts.begin();
	while (it != stmts.end()) {
		if ((*it)->static_offset != offset) {
			it = stmts.erase(it);	
		} else {
			it = stmts.erase(it);
			break;
		}
	}		
}

static std::vector<block::stmt::Ptr> trim_common_from_back(block::stmt::Ptr ast1, block::stmt::Ptr ast2) {
	std::vector<block::stmt::Ptr> trimmed_stmts;
	std::vector<block::stmt::Ptr> &ast1_stmts = block::to<block::stmt_block>(ast1)->stmts;
	std::vector<block::stmt::Ptr> &ast2_stmts = block::to<block::stmt_block>(ast2)->stmts;
	while(1) {
		if (ast1_stmts.back()->static_offset != ast2_stmts.back()->static_offset) {
			break;
		}
		block::stmt::Ptr trimmed_stmt = ast1_stmts.back();
		ast1_stmts.pop_back();
		ast2_stmts.pop_back();		
		trimmed_stmts.push_back(trimmed_stmt);	
	}
	std::reverse(trimmed_stmts.begin(), trimmed_stmts.end());
	return trimmed_stmts;
}
block::stmt::Ptr builder_context::extract_ast_from_function(ast_function_type function, std::vector<bool> b) {

	current_block_stmt = std::make_shared<block::stmt_block>();
	assert(current_block_stmt != nullptr);
	ast = current_block_stmt;
	var_name_counter = 0;
	bool_vector = b;
	
	current_function = function;

	block::stmt::Ptr ret_ast;
	try {
		function(this);
		ret_ast = extract_ast();

	} catch (OutOfBoolsException &e) {
		
		commit_uncommitted();
		
		block::expr_stmt::Ptr last_stmt = block::to<block::expr_stmt>(current_block_stmt->stmts.back());
		current_block_stmt->stmts.pop_back();
		
		block::expr::Ptr cond_expr = last_stmt->expr1;	

		builder_context true_context;
		std::vector<bool> true_bv;
		true_bv.push_back(true);
		std::copy(b.begin(), b.end(), std::back_inserter(true_bv));	
		block::stmt::Ptr true_ast = true_context.extract_ast_from_function(function, true_bv);	
		trim_ast_at_offset(true_ast, e.static_offset);


		builder_context false_context;
		std::vector<bool> false_bv;
		false_bv.push_back(false);
		std::copy(b.begin(), b.end(), std::back_inserter(false_bv));
		block::stmt::Ptr false_ast = false_context.extract_ast_from_function(function, false_bv);
		trim_ast_at_offset(false_ast, e.static_offset);
		std::vector<block::stmt::Ptr> trimmed_stmts = trim_common_from_back(true_ast, false_ast);
		
		block::if_stmt::Ptr new_if_stmt = std::make_shared<block::if_stmt>();
		new_if_stmt->context = this;
		new_if_stmt->static_offset = e.static_offset;
		
		new_if_stmt->cond = cond_expr;
		new_if_stmt->then_stmt = true_ast;
		new_if_stmt->else_stmt = false_ast;
		
		current_block_stmt->stmts.push_back(new_if_stmt);			
		
		std::copy(trimmed_stmts.begin(), trimmed_stmts.end(), std::back_inserter(current_block_stmt->stmts));
		ret_ast = ast;
	}
	
	ast = current_block_stmt = nullptr;
	return ret_ast;	
}

}

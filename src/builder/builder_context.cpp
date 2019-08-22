#include "builder/builder_context.h"
#include "builder/exceptions.h"
#include "util/tracer.h"
#include <algorithm>
#include "blocks/var_namer.h"
#include "blocks/label_inserter.h"
#include "blocks/loop_finder.h"

namespace builder {
builder_context* builder_context::current_builder_context = nullptr;
void builder_context::add_stmt_to_current_block(block::stmt::Ptr s) {
	if (current_label != "") {
		s->annotation = current_label;
		current_label = "";
	}
	if (!s->static_offset.is_empty() && is_visited_tag(s->static_offset) > 0) {
		throw LoopBackException(s->static_offset);
	}
	visited_offsets.push_back(s->static_offset);
	current_block_stmt->stmts.push_back(s);
}
tracer::tag get_offset_in_function(builder_context::ast_function_type _function) {
	tracer::tag offset = tracer::get_offset_in_function_impl(_function, builder_context::current_builder_context);
	return offset;
}
builder_context::builder_context() {
	current_block_stmt = nullptr;
	ast = nullptr;
}
bool builder_context::is_visited_tag(tracer::tag &new_tag) {
	for (int i = 0; i < visited_offsets.size(); i++) {
		if (visited_offsets[i] == new_tag)
			return true;
	}
	return false;
}
void builder_context::erase_tag(tracer::tag &erase_tag) {
	std::vector<tracer::tag> new_tags;
	for (int i = 0; i < visited_offsets.size(); i++) {
		if (visited_offsets[i] != erase_tag)
			new_tags.push_back(visited_offsets[i]);
	}
	visited_offsets = new_tags;
}
void builder_context::commit_uncommitted(void) {
	for (auto block_ptr: uncommitted_sequence) {
		block::expr_stmt::Ptr s = std::make_shared<block::expr_stmt>();
		assert(block::isa<block::expr>(block_ptr));
		s->static_offset = block_ptr->static_offset;
		s->expr1 = block::to<block::expr>(block_ptr);
		assert(current_block_stmt != nullptr);
		add_stmt_to_current_block(s);
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

bool get_next_bool_from_context(builder_context *context, block::expr::Ptr expr) {	
	if (context->bool_vector.size() == 0) {
		tracer::tag offset = expr->static_offset; 
		throw OutOfBoolsException(offset);
	}
	bool ret_val = context->bool_vector.back();
	context->bool_vector.pop_back();
	return ret_val; 
}


static void trim_ast_at_offset(block::stmt::Ptr ast, tracer::tag offset) {
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
	if (ast1_stmts.size() > 0 && ast2_stmts.size() > 0) {
		while(1) {
			
			if (ast1_stmts.back()->static_offset != ast2_stmts.back()->static_offset) {
				break;
			}
			if (ast1_stmts.back()->static_offset.is_empty()) {
				// The only possibility is that these two are goto statements. Gotos are same only if they are going to the same label
				assert(block::isa<block::goto_stmt>(ast1_stmts.back()));
				assert(block::isa<block::goto_stmt>(ast2_stmts.back()));
				block::goto_stmt::Ptr gt1 = block::to<block::goto_stmt>(ast1_stmts.back());
				block::goto_stmt::Ptr gt2 = block::to<block::goto_stmt>(ast2_stmts.back());
				if (gt1->temporary_label_number != gt2->temporary_label_number)
					break;
			}
			block::stmt::Ptr trimmed_stmt = ast1_stmts.back();
			ast1_stmts.pop_back();
			ast2_stmts.pop_back();		
			trimmed_stmts.push_back(trimmed_stmt);	
		}
	}
	std::reverse(trimmed_stmts.begin(), trimmed_stmts.end());
	return trimmed_stmts;
}
block::stmt::Ptr builder_context::extract_ast_from_function(ast_function_type function) {
	std::vector<bool> b;
	block::stmt::Ptr ast = extract_ast_from_function_internal(function, b);
	
	block::var_namer namer;
	namer.ast = ast;
	ast->accept(&namer);	

	block::label_collector collector;
	ast->accept(&collector);

	block::label_creator creator;
	creator.collected_labels = collector.collected_labels;
	ast->accept(&creator);

	block::label_inserter inserter;
	inserter.offset_to_label = creator.offset_to_label;
	ast->accept(&inserter);


	block::loop_finder finder;
	finder.ast = ast;
	ast->accept(&finder);

	return ast;
}
block::stmt::Ptr builder_context::extract_ast_from_function_internal(ast_function_type function, std::vector<bool> b) {
	
	current_block_stmt = std::make_shared<block::stmt_block>();
	current_block_stmt->static_offset.clear();
	assert(current_block_stmt != nullptr);
	ast = current_block_stmt;
	bool_vector = b;

	current_function = function;

	block::stmt::Ptr ret_ast;
	try {
		current_builder_context = this;
		function();
		
		ret_ast = extract_ast();
		current_builder_context = nullptr;

	} catch (OutOfBoolsException &e) {
		
		commit_uncommitted();
		current_builder_context = nullptr;
		
		block::expr_stmt::Ptr last_stmt = block::to<block::expr_stmt>(current_block_stmt->stmts.back());
		current_block_stmt->stmts.pop_back();
		erase_tag(e.static_offset);
		
		block::expr::Ptr cond_expr = last_stmt->expr1;	

		builder_context true_context;
		std::vector<bool> true_bv;
		true_bv.push_back(true);
		std::copy(b.begin(), b.end(), std::back_inserter(true_bv));	
		block::stmt_block::Ptr true_ast = block::to<block::stmt_block>(true_context.extract_ast_from_function_internal(function, true_bv));
		trim_ast_at_offset(true_ast, e.static_offset);


		builder_context false_context;
		std::vector<bool> false_bv;
		false_bv.push_back(false);
		std::copy(b.begin(), b.end(), std::back_inserter(false_bv));
		block::stmt_block::Ptr false_ast = block::to<block::stmt_block>(false_context.extract_ast_from_function_internal(function, false_bv));
		trim_ast_at_offset(false_ast, e.static_offset);



		std::vector<block::stmt::Ptr> trimmed_stmts = trim_common_from_back(true_ast, false_ast);
		
		


		block::if_stmt::Ptr new_if_stmt = std::make_shared<block::if_stmt>();
		new_if_stmt->annotation = last_stmt->annotation;
		new_if_stmt->static_offset = e.static_offset;
		
		new_if_stmt->cond = cond_expr;
		new_if_stmt->then_stmt = true_ast;
		new_if_stmt->else_stmt = false_ast;
		
		add_stmt_to_current_block(new_if_stmt);
		
		std::copy(trimmed_stmts.begin(), trimmed_stmts.end(), std::back_inserter(current_block_stmt->stmts));
		ret_ast = ast;
	} catch (LoopBackException &e) {
		current_builder_context = nullptr;
		
		block::goto_stmt::Ptr goto_stmt = std::make_shared<block::goto_stmt>();
		goto_stmt->static_offset.clear();
		goto_stmt->temporary_label_number = e.static_offset;
		
		add_stmt_to_current_block(goto_stmt);
		ret_ast = ast;	
	}
	
	ast = current_block_stmt = nullptr;
	return ret_ast;	
}

}

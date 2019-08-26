#include "builder/builder_context.h"
#include "builder/exceptions.h"
#include "util/tracer.h"
#include <algorithm>
#include "blocks/var_namer.h"
#include "blocks/label_inserter.h"
#include "blocks/loop_finder.h"
#include "builder/builder.h"

namespace builder {
builder_context* builder_context::current_builder_context = nullptr;
void builder_context::add_stmt_to_current_block(block::stmt::Ptr s, bool check_for_conflicts) {
	if (current_label != "") {
		s->annotation = current_label;
		current_label = "";
	}
	if (!s->static_offset.is_empty() && is_visited_tag(s->static_offset) > 0) {
		
		throw LoopBackException(s->static_offset);
	}
	std::string tag_string = s->static_offset.stringify();
	if (memoized_tags->map.find(tag_string) != memoized_tags->map.end() && check_for_conflicts && bool_vector.size() == 0) {
		// This tag has been seen on some other execution. We can reuse. 
		// First find the tag - 

		block::stmt_block::Ptr parent = memoized_tags->map[tag_string];
		int i = 0; 
		for (i = 0; i < parent->stmts.size(); i++) {
			if (parent->stmts[i]->static_offset == s->static_offset) 
				break;
		}
		// Special case of stmt expr and if_stmt
		if (block::isa<block::expr_stmt>(s) && block::isa<block::if_stmt>(parent->stmts[i])){
			block::if_stmt::Ptr p_stmt = block::to<block::if_stmt>(parent->stmts[i]);
			block::expr_stmt::Ptr expr = block::to<block::expr_stmt>(s);
			if (p_stmt->cond->is_same(expr->expr1))
				throw MemoizationException(s->static_offset, parent, i);
				
		}
		if (parent->stmts[i]->is_same(s))
			throw MemoizationException(s->static_offset, parent, i);
			
	}
	visited_offsets.insert(s->static_offset.stringify());
	current_block_stmt->stmts.push_back(s);
}
tracer::tag get_offset_in_function(builder_context::ast_function_type _function) {
	tracer::tag offset = tracer::get_offset_in_function_impl(_function, builder_context::current_builder_context);
	return offset;
}
builder_context::~builder_context() {
	for (int i = 0; i < assume_variables.size(); i++) {
		delete assume_variables[i];
	}
}
bool builder_context::is_visited_tag(tracer::tag &new_tag) {
	if (visited_offsets.find(new_tag.stringify()) != visited_offsets.end())
		return true;
	return false;
}
void builder_context::erase_tag(tracer::tag &erase_tag) {
	visited_offsets.erase(erase_tag.stringify());
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
	context->commit_uncommitted();
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
	std::vector<block::stmt::Ptr> new_stmts;
	
	int i;
	for (i = 0; i < stmts.size(); i++) {
		if (stmts[i]->static_offset == offset)
			break;
	}
	for (i++; i < stmts.size(); i++) {
		new_stmts.push_back(stmts[i]);
	}
	top_level_block->stmts = new_stmts;
}

static std::vector<block::stmt::Ptr> trim_common_from_back(block::stmt::Ptr ast1, block::stmt::Ptr ast2) {
	std::vector<block::stmt::Ptr> trimmed_stmts;
	std::vector<block::stmt::Ptr> &ast1_stmts = block::to<block::stmt_block>(ast1)->stmts;
	std::vector<block::stmt::Ptr> &ast2_stmts = block::to<block::stmt_block>(ast2)->stmts;
	if (ast1_stmts.size() > 0 && ast2_stmts.size() > 0) {
		while(1) {
			if (ast1_stmts.size() == 0 || ast2_stmts.size() == 0)
				break;
			if (!ast1_stmts.back()->is_same(ast2_stmts.back()))
				break;	
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
		
		current_builder_context = nullptr;
		
		block::expr_stmt::Ptr last_stmt = block::to<block::expr_stmt>(current_block_stmt->stmts.back());
		current_block_stmt->stmts.pop_back();
		erase_tag(e.static_offset);
		
		block::expr::Ptr cond_expr = last_stmt->expr1;	

		builder_context true_context(memoized_tags);
		std::vector<bool> true_bv;
		true_bv.push_back(true);
		std::copy(b.begin(), b.end(), std::back_inserter(true_bv));	
		block::stmt_block::Ptr true_ast = block::to<block::stmt_block>(true_context.extract_ast_from_function_internal(function, true_bv));


		builder_context false_context(memoized_tags);
		std::vector<bool> false_bv;
		false_bv.push_back(false);
		std::copy(b.begin(), b.end(), std::back_inserter(false_bv));
		block::stmt_block::Ptr false_ast = block::to<block::stmt_block>(false_context.extract_ast_from_function_internal(function, false_bv));

		trim_ast_at_offset(true_ast, e.static_offset);
		trim_ast_at_offset(false_ast, e.static_offset);



		std::vector<block::stmt::Ptr> trimmed_stmts = trim_common_from_back(true_ast, false_ast);
		
		


		block::if_stmt::Ptr new_if_stmt = std::make_shared<block::if_stmt>();
		new_if_stmt->annotation = last_stmt->annotation;
		new_if_stmt->static_offset = e.static_offset;
		
		new_if_stmt->cond = cond_expr;
		new_if_stmt->then_stmt = true_ast;
		new_if_stmt->else_stmt = false_ast;
		
		add_stmt_to_current_block(new_if_stmt, false);
		
		std::copy(trimmed_stmts.begin(), trimmed_stmts.end(), std::back_inserter(current_block_stmt->stmts));
		ret_ast = ast;
	} catch (LoopBackException &e) {
		current_builder_context = nullptr;
		
		block::goto_stmt::Ptr goto_stmt = std::make_shared<block::goto_stmt>();
		goto_stmt->static_offset.clear();
		goto_stmt->temporary_label_number = e.static_offset;
		
		add_stmt_to_current_block(goto_stmt, false);
		ret_ast = ast;	
	} catch (MemoizationException &e) {
		for (int i = e.child_id; i < e.parent->stmts.size(); i++) {
			add_stmt_to_current_block(e.parent->stmts[i], false);
		}		
		ret_ast = ast;
	}
	

	// Update the memoized table with the stmt block we just created 
	for (int i = 0; i < current_block_stmt->stmts.size(); i++) {
		block::stmt::Ptr s = current_block_stmt->stmts[i];
		memoized_tags->map[s->static_offset.stringify()] = current_block_stmt;
	}

	ast = current_block_stmt = nullptr;
	return ret_ast;	
}

}

#include "builder/builder_context.h"
#include "blocks/for_loop_finder.h"
#include "blocks/if_switcher.h"
#include "blocks/label_inserter.h"
#include "blocks/loop_finder.h"
#include "blocks/loop_roll.h"
#include "blocks/rce.h"
#include "blocks/sub_expr_cleanup.h"
#include "blocks/generic_checker.h"
#include "blocks/var_namer.h"
#include "builder/builder.h"
#include "builder/dyn_var.h"
#include "builder/exceptions.h"
#include "util/tracer.h"
#include <algorithm>

namespace builder {


static void trim_ast_at_offset(block::stmt::Ptr ast, tracer::tag offset) {
	return;
	block::stmt_block::Ptr top_level_block = block::to<block::stmt_block>(ast);
	std::vector<block::stmt::Ptr> &stmts = top_level_block->stmts;
	std::vector<block::stmt::Ptr> new_stmts;

	unsigned int i;
	for (i = 0; i < stmts.size(); i++) {
		if (stmts[i]->static_offset == offset)
			break;
	}
	for (i++; i < stmts.size(); i++) {
		new_stmts.push_back(stmts[i]);
	}
	top_level_block->stmts = new_stmts;
}

static std::pair<std::vector<block::stmt::Ptr>, std::vector<block::stmt::Ptr>>
trim_common_from_back(block::stmt::Ptr ast1, block::stmt::Ptr ast2) {

	std::vector<block::stmt::Ptr> trimmed_stmts;
	std::vector<block::stmt::Ptr> &ast1_stmts = block::to<block::stmt_block>(ast1)->stmts;
	std::vector<block::stmt::Ptr> &ast2_stmts = block::to<block::stmt_block>(ast2)->stmts;

	std::vector<block::stmt::Ptr> split_decls;

	if (ast1_stmts.size() > 0 && ast2_stmts.size() > 0) {
		while (1) {
			if (ast1_stmts.size() == 0 || ast2_stmts.size() == 0)
				break;
			if (ast1_stmts.back()->static_offset != ast2_stmts.back()->static_offset ||
			    !ast1_stmts.back()->is_same(ast2_stmts.back())) {
				// There is a special case where there could be
				// an if stmt with same body but different
				// conditions We handle that by splitting the
				// condition from the if stmt using a variable
				if (block::isa<block::if_stmt>(ast1_stmts.back()) &&
				    block::isa<block::if_stmt>(ast2_stmts.back())) {

					block::if_stmt::Ptr if1 = block::to<block::if_stmt>(ast1_stmts.back());
					block::if_stmt::Ptr if2 = block::to<block::if_stmt>(ast2_stmts.back());
					if (if1->needs_splitting(if2)) {
						ast1_stmts.pop_back();
						ast2_stmts.pop_back();

						block::var::Ptr cond_var = std::make_shared<block::var>();
						cond_var->var_type = type_extractor<int>::extract_type();
						cond_var->static_offset = tracer::get_unique_tag();

						block::decl_stmt::Ptr decl_stmt = std::make_shared<block::decl_stmt>();
						decl_stmt->static_offset = if1->static_offset;

						decl_stmt->decl_var = cond_var;
						decl_stmt->init_expr = nullptr;

						split_decls.push_back(decl_stmt);

						block::expr_stmt::Ptr stmt1 = std::make_shared<block::expr_stmt>();
						stmt1->static_offset = if1->static_offset;
						block::assign_expr::Ptr assign1 =
						    std::make_shared<block::assign_expr>();
						assign1->static_offset = if1->static_offset;
						block::var_expr::Ptr varexpr1 = std::make_shared<block::var_expr>();
						varexpr1->static_offset = if1->static_offset;
						varexpr1->var1 = cond_var;
						assign1->var1 = varexpr1;
						assign1->expr1 = if1->cond;
						stmt1->expr1 = assign1;
						ast1_stmts.push_back(stmt1);

						block::expr_stmt::Ptr stmt2 = std::make_shared<block::expr_stmt>();
						stmt2->static_offset = if2->static_offset;
						block::assign_expr::Ptr assign2 =
						    std::make_shared<block::assign_expr>();
						assign2->static_offset = if2->static_offset;
						block::var_expr::Ptr varexpr2 = std::make_shared<block::var_expr>();
						varexpr2->static_offset = if2->static_offset;
						varexpr2->var1 = cond_var;
						assign2->var1 = varexpr2;
						assign2->expr1 = if2->cond;
						stmt2->expr1 = assign2;
						ast2_stmts.push_back(stmt2);

						block::var_expr::Ptr varexpr3 = std::make_shared<block::var_expr>();
						varexpr3->static_offset = if2->static_offset;
						varexpr3->var1 = cond_var;
						if1->cond = varexpr3;
						trimmed_stmts.push_back(if1);
						continue;

					} else {
						break;
					}
				}
				break;
			}
			if (ast1_stmts.back()->static_offset.is_empty()) {
				// The only possibility is that these two are
				// goto statements. Gotos are same only if they
				// are going to the same label
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
	// Handle a special case where one of the branch ends in a goto
	// In this case everything from the second branch can be safely added to
	// the common part This has to be checked only in the end because gotos
	// can appear on both the sides and should be trimmed of before

	// Also allow this optimization if one of the branch ends in return
	if (ast1_stmts.size() != 0 && ast2_stmts.size() != 0) {
		if (block::isa<block::goto_stmt>(ast1_stmts.back()) ||
		    block::isa<block::return_stmt>(ast1_stmts.back())) {
			while (ast2_stmts.size() > 0) {
				block::stmt::Ptr trimmed_stmt = ast2_stmts.back();
				ast2_stmts.pop_back();
				trimmed_stmts.push_back(trimmed_stmt);
			}
		} else if (block::isa<block::goto_stmt>(ast2_stmts.back()) ||
			   block::isa<block::return_stmt>(ast2_stmts.back())) {
			while (ast1_stmts.size() > 0) {
				block::stmt::Ptr trimmed_stmt = ast1_stmts.back();
				ast1_stmts.pop_back();
				trimmed_stmts.push_back(trimmed_stmt);
			}
		}
	}

	std::reverse(trimmed_stmts.begin(), trimmed_stmts.end());
	return {trimmed_stmts, split_decls};
}
/*
static void builder_context::reset_for_nd_failure() {
	// Clear all shared_state
	memoized_tags->map.clear();
	// Clear per run state if any
	uncommitted_sequence.clear();
	ast = nullptr;
	current_block_stmt = nullptr;
	bool_vector.clear();
	visited_offsets.clear();
	expr_sequence.clear();
	expr_counter = 0;
	current_label = "";
	static_var_tuples.clear();
	deferred_static_var_tuples.clear();
	// Increment creation counter since we are running again
	debug_creation_counter++;
}
*/


void builder_context::extract_function_ast_impl(invocation_state* i_state) {

#ifndef ENABLE_D2X
	if (enable_d2x)
		assert(false && "D2X support cannot be enabled without the ENABLE_D2X build option");
#endif
	block::stmt::Ptr ast = nullptr;
	// Repeat till ND vars are happy
	while (1) {
		try {
			// Allocate one execution_state for this ND run
			execution_state e_state (i_state);
			// Allocate one run_state, rest will be allocated by the recursive calls
			run_state r_state (&e_state, i_state);
			ast = extract_ast_from_run(&r_state);
		} catch (NonDeterministicFailureException &e) {
			continue;
		}
		break;
	}

	// Before making any changes, untangle the whole AST
	ast = clone(ast);
	
	// Make sure any generics haven't been left 
	// unspecialized
	block::generic_null_checker checker;
	ast->accept(&checker);

	block::var_namer::name_vars(ast);

	block::label_collector collector;
	ast->accept(&collector);

	block::label_creator creator;
	creator.collected_labels = collector.collected_labels;
	ast->accept(&creator);

	block::label_inserter inserter;
	inserter.backup_offset_to_label = creator.offset_to_label;
	inserter.feature_unstructured = feature_unstructured;
	ast->accept(&inserter);

	// At this point it is safe to remove statements that are
	// marked for deletion
	block::sub_expr_cleanup cleaner;
	ast->accept(&cleaner);

	if (!feature_unstructured) {

		block::basic_block::cfg_block BBs = generate_basic_blocks(block::to<block::stmt_block>(ast));
		
		block::loop_finder finder;
		finder.ast = ast;
		ast->accept(&finder);

		block::for_loop_finder for_finder;
		for_finder.ast = ast;
		ast->accept(&for_finder);
	}

	block::if_switcher switcher;
	ast->accept(&switcher);

	
	// Run RCE after loop finder
	// since RCE does rely on loops being detected
	// If labels are still kept around, RCE cannot be as aggressive 
	// since it has to consider the worst case
	if (run_rce) {
		block::eliminate_redundant_vars(ast);
	}

	block::loop_roll_finder loop_roll_finder;
	ast->accept(&loop_roll_finder);


	i_state->generated_func_decl->body = ast;	
}
block::stmt::Ptr builder_context::extract_ast_from_run(run_state* r_state) {
	r_state->current_stmt_block = std::make_shared<block::stmt_block>();
	block::stmt_block::Ptr ast = r_state->current_stmt_block;

	// A new run is starting, clear the parent stack
	// for identifying nested members. This is because a run can end mid construction
	if (parents_stack != nullptr) {
		parents_stack->clear();
	}

	block::stmt_block::Ptr ret_ast;

	std::vector<bool> bool_vector_copy = r_state->bool_vector;

	try {
		run_state::current_run_state = r_state;
		// function();
		lambda_wrapper(r_state->i_state->invocation_function);
		r_state->commit_uncommitted();
		ret_ast = ast;
		run_state::current_run_state = nullptr;

	} catch (OutOfBoolsException &e) {

		run_state::current_run_state = nullptr;

		block::expr_stmt::Ptr last_stmt = block::to<block::expr_stmt>(r_state->current_stmt_block->stmts.back());
		r_state->current_stmt_block->stmts.pop_back();

		block::expr::Ptr cond_expr = last_stmt->expr1;
	
		// Establish two run_states
		run_state true_r_state(r_state->e_state, r_state->i_state);
		// Only copy over the expr_sequence since it is part of the r_state that 
		// just terminated
		true_r_state.cached_expr_sequence = r_state->cached_expr_sequence;
		true_r_state.bool_vector.push_back(true);
		true_r_state.visited_offsets = r_state->visited_offsets;
		std::copy(bool_vector_copy.begin(), bool_vector_copy.end(), std::back_inserter(true_r_state.bool_vector));

		// Establish two run_states
		run_state false_r_state(r_state->e_state, r_state->i_state);
		false_r_state.visited_offsets = r_state->visited_offsets;
		// Only copy over the expr_sequence since it is part of the r_state that 
		// just terminated
		false_r_state.cached_expr_sequence = r_state->cached_expr_sequence;
		false_r_state.bool_vector = true_r_state.bool_vector;
		false_r_state.bool_vector[0] = false;

		block::stmt_block::Ptr true_ast = block::to<block::stmt_block>(extract_ast_from_run(&true_r_state));
		block::stmt_block::Ptr false_ast = block::to<block::stmt_block>(extract_ast_from_run(&false_r_state));

		trim_ast_at_offset(true_ast, e.static_offset);
		trim_ast_at_offset(false_ast, e.static_offset);

		std::pair<std::vector<block::stmt::Ptr>, std::vector<block::stmt::Ptr>> trim_pair =
		    trim_common_from_back(true_ast, false_ast);

		std::vector<block::stmt::Ptr> trimmed_stmts = trim_pair.first;
		std::vector<block::stmt::Ptr> split_decls = trim_pair.second;

		r_state->erase_tag(e.static_offset);

		block::if_stmt::Ptr new_if_stmt = std::make_shared<block::if_stmt>();
		new_if_stmt->annotation = last_stmt->annotation;
		new_if_stmt->static_offset = e.static_offset;

		new_if_stmt->cond = cond_expr;
		new_if_stmt->then_stmt = true_ast;
		new_if_stmt->else_stmt = false_ast;

		for (auto stmt : split_decls)
			r_state->add_stmt_to_current_block(stmt, false);
		r_state->add_stmt_to_current_block(new_if_stmt, false);

		std::copy(trimmed_stmts.begin(), trimmed_stmts.end(), std::back_inserter(r_state->current_stmt_block->stmts));

		ret_ast = ast;
	} catch (LoopBackException &e) {
		run_state::current_run_state = nullptr;
		block::goto_stmt::Ptr goto_stmt = std::make_shared<block::goto_stmt>();
		goto_stmt->static_offset.clear();
		goto_stmt->temporary_label_number = e.static_offset;

		r_state->add_stmt_to_current_block(goto_stmt, false);
		ret_ast = ast;
	} catch (MemoizationException &e) {
		run_state::current_run_state = nullptr;
		if (feature_unstructured) {
			// Instead of copying statements to the current block, we will just insert a goto
			block::goto_stmt::Ptr goto_stmt = std::make_shared<block::goto_stmt>();
			goto_stmt->static_offset.clear();
			goto_stmt->temporary_label_number = e.static_offset;
			r_state->add_stmt_to_current_block(goto_stmt, false);
		} else {
			for (unsigned int i = e.child_id; i < e.parent->stmts.size(); i++) {
				if (block::isa<block::goto_stmt>(e.parent->stmts[i])) {
					block::goto_stmt::Ptr goto_stmt = std::make_shared<block::goto_stmt>();
					goto_stmt->static_offset.clear();
					goto_stmt->temporary_label_number = block::to<block::goto_stmt>(e.parent->stmts[i])->temporary_label_number;
					r_state->add_stmt_to_current_block(goto_stmt, false);
				}
				else {
					r_state->add_stmt_to_current_block(e.parent->stmts[i], false);
				}
			}
		}
		ret_ast = ast;
	}
	run_state::current_run_state = nullptr;

	// Update the memoized table with the stmt block we just created
	for (unsigned int i = 0; i < r_state->current_stmt_block->stmts.size(); i++) {
		block::stmt::Ptr s = r_state->current_stmt_block->stmts[i];
		// If any of the statements are if conditions, remove the
		// internal statements from the table
		// This is required because of the way we do memoization.
		// We just store a pointer to the stmt_block that has the statement.
		// In case of if stmt, the then and else branches don't have all the required
		// statements. There might be statements AFTER the if which will not be
		// included in the memoization result.
		// But, if we are using feature_unstructured, we don't really care about
		// the statements, just the existence of the statement. So we will reset
		// the memoization table to point to the new blocks around
		if (block::isa<block::if_stmt>(s)) {
			block::if_stmt::Ptr if1 = block::to<block::if_stmt>(s);
			assert(block::isa<block::stmt_block>(if1->then_stmt));
			assert(block::isa<block::stmt_block>(if1->else_stmt));
			for (auto &stmt : block::to<block::stmt_block>(if1->then_stmt)->stmts) {
				auto it = r_state->e_state->memoized_tags.find(stmt->static_offset);
				if (it != r_state->e_state->memoized_tags.end())
					r_state->e_state->memoized_tags.erase(it);

				if (feature_unstructured) {
					auto pblock = block::to<block::stmt_block>(if1->then_stmt);
					r_state->e_state->memoized_tags[stmt->static_offset] = pblock;
				}
			}
			for (auto &stmt : block::to<block::stmt_block>(if1->else_stmt)->stmts) {
				auto it = r_state->e_state->memoized_tags.find(stmt->static_offset);
				if (it != r_state->e_state->memoized_tags.end())
					r_state->e_state->memoized_tags.erase(it);
				if (feature_unstructured) {
					auto pblock = block::to<block::stmt_block>(if1->else_stmt);
					r_state->e_state->memoized_tags[stmt->static_offset] = pblock;
				}
			}
		}
		r_state->e_state->memoized_tags[s->static_offset] = r_state->current_stmt_block;
	}

	ast = r_state->current_stmt_block = nullptr;
	return ret_ast;
}

} // namespace builder

#include "blocks/loop_finder.h"
#include <algorithm>
namespace block {

static void ensure_back_has_goto(stmt_block::Ptr a, label::Ptr label_detect, std::vector<stmt_block::Ptr> &parents) {

	if (a->stmts.size() == 0) {
		parents.push_back(a);
		return;
	}
	stmt::Ptr last_stmt = a->stmts.back();
	if (isa<if_stmt>(last_stmt)) {
		// For ifs don't add unnecessary common ends
		if_stmt::Ptr if_stmt_ptr = to<if_stmt>(last_stmt);

		stmt_block::Ptr then_block = to<stmt_block>(if_stmt_ptr->then_stmt);
		stmt_block::Ptr else_block = to<stmt_block>(if_stmt_ptr->else_stmt);
		std::vector<stmt_block::Ptr> if_parents;
		ensure_back_has_goto(then_block, label_detect, if_parents);
		ensure_back_has_goto(else_block, label_detect, if_parents);
		if (if_parents.size() == 2 && if_parents[0] == then_block && if_parents[1] == else_block && false) {
			parents.push_back(a);
		} else {
			for (unsigned int i = 0; i < if_parents.size(); i++) {
				parents.push_back(if_parents[i]);
			}
		}
	} else if (isa<goto_stmt>(last_stmt) && to<goto_stmt>(last_stmt)->label1 == label_detect) {
		a->stmts.pop_back();
	} else if (isa<goto_stmt>(last_stmt) && to<goto_stmt>(last_stmt)->label1 != label_detect) {
		parents.push_back(a);
	} else if (isa<break_stmt>(last_stmt)) {
		assert(false);
	} else {
		parents.push_back(a);
	}
	return;
}

static void insert_continues(stmt_block::Ptr a, label::Ptr label_detect, std::vector<stmt_block::Ptr> &collect) {
	for (auto stmt : a->stmts) {
		if (isa<if_stmt>(stmt)) {
			if_stmt::Ptr if_stmt_ptr = to<if_stmt>(stmt);
			stmt_block::Ptr then_block = to<stmt_block>(if_stmt_ptr->then_stmt);
			stmt_block::Ptr else_block = to<stmt_block>(if_stmt_ptr->else_stmt);
			insert_continues(then_block, label_detect, collect);
			insert_continues(else_block, label_detect, collect);
		}
	}
	if (a->stmts.size() > 0 && isa<goto_stmt>(a->stmts.back())) {
		if (to<goto_stmt>(a->stmts.back())->label1 == label_detect) {
			a->stmts.pop_back();
			continue_stmt::Ptr cont = std::make_shared<continue_stmt>();
			a->stmts.push_back(cont);
			collect.push_back(a);
		}
	}
}
static void insert_breaks(stmt_block::Ptr a, label::Ptr label_detect, std::vector<stmt_block::Ptr> &parents) {
	for (auto stmt : a->stmts) {
		if (isa<if_stmt>(stmt)) {
			if_stmt::Ptr if_stmt_ptr = to<if_stmt>(stmt);
			stmt_block::Ptr then_block = to<stmt_block>(if_stmt_ptr->then_stmt);
			stmt_block::Ptr else_block = to<stmt_block>(if_stmt_ptr->else_stmt);
			insert_breaks(then_block, label_detect, parents);
			insert_breaks(else_block, label_detect, parents);
		}
	}

	if (a->stmts.size() == 0)
		return;

	if (a->stmts.size() > 0 && isa<goto_stmt>(a->stmts.back())) {
		if (to<goto_stmt>(a->stmts.back())->label1 == label_detect) {
			return;
		}
	}
	// This needs a break because it doesn't continue
	// But before we add, we should check if parents already has it

	if (a->stmts.size() > 0 && isa<goto_stmt>(a->stmts.back())) {
		for (auto stmt : parents) {
			if (stmt == a)
				return;
		}
		parents.push_back(a);
	}
}

void continue_finder::visit(continue_stmt::Ptr) {
	has_continue = true;
}

static bool check_last_choppable(std::vector<stmt_block::Ptr> &parents) {
	// Check if everyone has atleast one stmt
	for (unsigned int i = 0; i < parents.size(); i++) {
		if (parents[i]->stmts.size() == 0)
			return false;
	}

	stmt::Ptr last_stmt = parents[0]->stmts.back();
	continue_finder finder;
	last_stmt->accept(&finder);
	if (finder.has_continue)
		return false;

	if (parents.size() == 1)
		return true;

	tracer::tag first_tag = parents[0]->stmts.back()->static_offset;
	for (unsigned int i = 1; i < parents.size(); i++) {
		if (parents[i]->stmts.back()->static_offset != first_tag)
			return false;
	}
	return true;
}
static void trim_from_parents(std::vector<stmt_block::Ptr> &parents, std::vector<stmt::Ptr> &trimmed) {

	// First check if the ends are all same
	if (check_last_choppable(parents)) {
		// Chop a stmt off of everyone
		stmt::Ptr chopped = parents[0]->stmts.back();
		for (unsigned int i = 0; i < parents.size(); i++) {
			parents[i]->stmts.pop_back();
		}
		trimmed.push_back(chopped);
		trim_from_parents(parents, trimmed);
	}
}

void loop_finder::visit(stmt_block::Ptr a) {

	// We do this inside out, first do the innermost loop

	// Visit the instructions normally
	for (auto stmt : a->stmts) {
		stmt->accept(this);
	}

	// Check if this block has a label
	while (1) {
		label_stmt::Ptr found_label = nullptr;
		for (auto stmt : a->stmts) {
			if (isa<label_stmt>(stmt)) {
				found_label = to<label_stmt>(stmt);
			}
		}
		if (found_label == nullptr)
			break;
		visit_label(found_label, a);
	}
}

static void merge_condition_with_loop(while_stmt::Ptr new_while) {
	// If the body of the while loop only has a single if condition and
	// the else part of the condition is just a break, fuse the if with
	// the loop

	if (to<stmt_block>(new_while->body)->stmts.size() == 1 &&
	    isa<if_stmt>(to<stmt_block>(new_while->body)->stmts[0])) {
		if_stmt::Ptr if_body = to<if_stmt>(to<stmt_block>(new_while->body)->stmts[0]);

		stmt::Ptr then_stmt = if_body->then_stmt;
		stmt::Ptr else_stmt = if_body->else_stmt;

		if (isa<stmt_block>(else_stmt) && to<stmt_block>(else_stmt)->stmts.size() == 1) {
			if (isa<break_stmt>(to<stmt_block>(else_stmt)->stmts[0])) {
				new_while->cond = if_body->cond;
				// new_while->body =
				// std::make_shared<stmt_block>();
				new_while->body = then_stmt;
				return;
			}
		}
		if (isa<stmt_block>(then_stmt) && to<stmt_block>(then_stmt)->stmts.size() == 1) {
			if (isa<break_stmt>(to<stmt_block>(then_stmt)->stmts[0])) {
				not_expr::Ptr new_cond = std::make_shared<not_expr>();
				new_cond->static_offset = if_body->cond->static_offset;
				new_cond->expr1 = if_body->cond;
				new_while->cond = new_cond;
				new_while->body = else_stmt;
				return;
			}
		}
	}
	// Other pattern is if the loops first statement is a if condition that
	// breaks
	if (isa<if_stmt>(to<stmt_block>(new_while->body)->stmts[0])) {
		if_stmt::Ptr if_body = to<if_stmt>(to<stmt_block>(new_while->body)->stmts[0]);
		stmt::Ptr then_stmt = if_body->then_stmt;

		if (isa<stmt_block>(then_stmt) && to<stmt_block>(then_stmt)->stmts.size() == 1) {
			if (isa<break_stmt>(to<stmt_block>(then_stmt)->stmts[0])) {
				not_expr::Ptr new_cond = std::make_shared<not_expr>();
				new_cond->static_offset = if_body->cond->static_offset;
				new_cond->expr1 = if_body->cond;
				new_while->cond = new_cond;
				auto new_body = std::make_shared<stmt_block>();
				for (unsigned int i = 1; i < to<stmt_block>(new_while->body)->stmts.size(); i++) {
					new_body->stmts.push_back(to<stmt_block>(new_while->body)->stmts[i]);
				}
				new_while->body = new_body;
				return;
			}
		}
	}
}

void loop_finder::visit_label(label_stmt::Ptr a, stmt_block::Ptr parent) {

	// First separate out the stmts before the loop begin
	std::vector<stmt::Ptr> stmts_before;
	std::vector<stmt::Ptr> stmts_in_body;
	std::vector<stmt::Ptr> stmts_after_body;

	stmt::Ptr last_stmt = nullptr;

	for (auto stmt : parent->stmts) {
		last_jump_finder jump_finder;
		jump_finder.jump_label = a->label1;
		stmt->accept(&jump_finder);
		if (jump_finder.has_jump_to == true)
			last_stmt = stmt;
	}

	if (last_stmt == nullptr) {
		// This label was created but has no jump.
		// this currently happens when two statements have the same tag
		// For now we will just delete this label
		std::vector<stmt::Ptr> new_stmts;
		for (auto stmt : parent->stmts) {
			if (stmt == a)
				continue;
			new_stmts.push_back(stmt);
		}
		parent->stmts = new_stmts;
		return;
	}

	std::vector<stmt::Ptr>::iterator stmt;
	for (stmt = parent->stmts.begin(); stmt != parent->stmts.end(); stmt++) {
		if (*stmt == a)
			break;
		stmts_before.push_back(*stmt);
	}
	stmt++;
	for (; stmt != parent->stmts.end(); stmt++) {
		stmts_in_body.push_back(*stmt);
		if (*stmt == last_stmt)
			break;
	}
	stmt++;
	for (; stmt != parent->stmts.end(); stmt++) {
		stmts_after_body.push_back(*stmt);
	}
	parent->stmts.clear();

	while_stmt::Ptr new_while = std::make_shared<while_stmt>();
	new_while->cond = std::make_shared<int_const>();
	to<int_const>(new_while->cond)->value = 1;
	new_while->body = std::make_shared<stmt_block>();
	to<stmt_block>(new_while->body)->stmts = stmts_in_body;

	std::vector<stmt_block::Ptr> parents;

	// Clean up all loops in this body
	loop_finder finder;
	finder.ast = new_while->body;
	new_while->body->accept(&finder);

	ensure_back_has_goto(to<stmt_block>(new_while->body), a->label1, parents);

	std::vector<stmt_block::Ptr> collects;

	insert_continues(to<stmt_block>(new_while->body), a->label1, collects);
	new_while->continue_blocks = collects;

	insert_breaks(to<stmt_block>(new_while->body), a->label1, parents);

	std::vector<stmt::Ptr> trimmed;

	if (parents.size() > 0)
		trim_from_parents(parents, trimmed);

	// Now push a break to the end of every parent
	for (stmt_block::Ptr block : parents) {

		// if (block->stmts.size() > 0 &&
		// isa<goto_stmt>(block->stmts.back())) continue;

		break_stmt::Ptr new_break = std::make_shared<break_stmt>();
		block->stmts.push_back(new_break);
	}

	std::reverse(trimmed.begin(), trimmed.end());

	merge_condition_with_loop(new_while);

	// Once we are happy with the loops, we have to make sure that this loop doesn't have any other jumps
	// If it does, we should pull them out. So outer loops can handle them
	outer_jump_finder outer_finder(loop_hook_counter);
	new_while->accept(&outer_finder);

	// For every control guard variable insert a initialization before the loop and the beginning of the loop

	std::vector<stmt::Ptr> new_body_stmts;
	std::vector<stmt::Ptr> guard_decl_stmts;
	std::vector<stmt::Ptr> guarded_jumps;
	for (auto guards : outer_finder.created_vars) {
		var::Ptr var1 = guards.first;

		auto var_expr1 = std::make_shared<var_expr>();
		var_expr1->var1 = var1;
		auto const_expr1 = std::make_shared<int_const>();
		const_expr1->value = 0;
		const_expr1->is_64bit = false;
		auto assign_expr1 = std::make_shared<assign_expr>();
		assign_expr1->var1 = var_expr1;
		assign_expr1->expr1 = const_expr1;

		auto expr_stmt1 = std::make_shared<expr_stmt>();
		expr_stmt1->expr1 = assign_expr1;

		new_body_stmts.push_back(expr_stmt1);

		auto var_decl1 = std::make_shared<decl_stmt>();
		var_decl1->decl_var = var1;
		var_decl1->init_expr = const_expr1;
		guard_decl_stmts.push_back(var_decl1);

		auto if_stmt1 = std::make_shared<if_stmt>();
		if_stmt1->else_stmt = std::make_shared<stmt_block>();
		auto stmt_block1 = std::make_shared<stmt_block>();
		if_stmt1->then_stmt = stmt_block1;
		stmt_block1->stmts.push_back(guards.second);

		auto var_expr2 = std::make_shared<var_expr>();
		var_expr2->var1 = var1;
		if_stmt1->cond = var_expr2;

		guarded_jumps.push_back(if_stmt1);
	}

	// Insert all the original statements
	for (auto stmt : to<stmt_block>(new_while->body)->stmts) {
		new_body_stmts.push_back(stmt);
	}
	to<stmt_block>(new_while->body)->stmts = new_body_stmts;

	// New while is ready to be inserted
	parent->stmts = stmts_before;
	// Insert the new guard decls we created
	for (auto stmt : guard_decl_stmts) {
		parent->stmts.push_back(stmt);
	}
	parent->stmts.push_back(new_while);
	// Insert the guaded jumps afer
	for (auto stmt : guarded_jumps) {
		parent->stmts.push_back(stmt);
	}
	for (auto stmt : trimmed) {
		parent->stmts.push_back(stmt);
	}
	for (auto stmt : stmts_after_body) {
		parent->stmts.push_back(stmt);
	}
}

void outer_jump_finder::visit(stmt_block::Ptr block) {
	// First visit all the statements normally
	block_visitor::visit(block);

	std::vector<stmt::Ptr> new_stmts;
	for (auto stmt : block->stmts) {
		if (isa<goto_stmt>(stmt)) {
			// We found a jump statement, this must escape this loop, otherwise it would have
			// been replaced with a continue
			// we should now create a new variable and assignment
			auto var1 = std::make_shared<var>();
			var1->var_name = "control_guard" + std::to_string(loop_hook_counter++);
			auto scalar_type1 = std::make_shared<scalar_type>();
			var1->var_type = scalar_type1;
			scalar_type1->scalar_type_id = scalar_type::INT_TYPE;
			auto var_expr1 = std::make_shared<var_expr>();
			var_expr1->var1 = var1;
			auto const_expr1 = std::make_shared<int_const>();
			const_expr1->value = 1;
			const_expr1->is_64bit = false;
			auto assign_expr1 = std::make_shared<assign_expr>();
			assign_expr1->var1 = var_expr1;
			assign_expr1->expr1 = const_expr1;

			auto expr_stmt1 = std::make_shared<expr_stmt>();
			expr_stmt1->expr1 = assign_expr1;

			new_stmts.push_back(expr_stmt1);

			created_vars.push_back(std::make_pair(var1, stmt));
		} else {
			new_stmts.push_back(stmt);
		}
	}
	block->stmts = new_stmts;
}

void last_jump_finder::visit(goto_stmt::Ptr a) {
	if (a->label1 == jump_label) {
		has_jump_to = true;
	}
}
} // namespace block

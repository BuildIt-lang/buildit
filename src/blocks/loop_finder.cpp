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
		if (if_parents.size() == 2 && if_parents[0] == then_block && if_parents[1] == else_block) {
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


static bool check_last_choppable(std::vector<stmt_block::Ptr> &parents) {
	// Check if everyone has atleast one stmt
	for (unsigned int i = 0; i < parents.size(); i++) {
		if (parents[i]->stmts.size() == 0)
			return false;
	}

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
	// Check if this block has a label
	while (1) {
		label_stmt::Ptr found_label = nullptr;
		for (auto stmt: a->stmts) {
			if (isa<label_stmt>(stmt)) {
				found_label = to<label_stmt>(stmt);
			}
		}	
		if (found_label == nullptr) 
			break;
		visit_label(found_label, a);	
	}
	// Once all labels are done, visit the instructions normally 
	for (auto stmt: a->stmts) {
		stmt->accept(this);
	}
}
void loop_finder::visit_label(label_stmt::Ptr a, stmt_block::Ptr parent) {
	
	// First separate out the stmts before the loop begin
	std::vector<stmt::Ptr> stmts_before;
	std::vector<stmt::Ptr> stmts_in_body;
	std::vector<stmt::Ptr> stmts_after_body;

	stmt::Ptr last_stmt = nullptr;

	for (auto stmt: parent->stmts) {
		last_jump_finder jump_finder;
		jump_finder.jump_label = a->label1;
		stmt->accept(&jump_finder);
		if (jump_finder.has_jump_to == true) 
			last_stmt = stmt;	
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

	//Clean up all loops in this body  
	loop_finder finder;
	finder.ast = new_while->body;
	new_while->body->accept(&finder);

	
	ensure_back_has_goto(to<stmt_block>(new_while->body), a->label1, parents);
	
	std::vector<stmt::Ptr> trimmed;
	if (parents.size() > 0)
		trim_from_parents(parents, trimmed);
	
	// Now push a break to the end of every parent
	for (stmt_block::Ptr block: parents) {
		
		//if (block->stmts.size() > 0 && isa<goto_stmt>(block->stmts.back()))
			//continue;
		
		break_stmt::Ptr new_break = std::make_shared<break_stmt>();
		block->stmts.push_back(new_break);
		
	}	
	
	std::reverse(trimmed.begin(), trimmed.end());
	parent->stmts = stmts_before;
	parent->stmts.push_back(new_while);

	for (auto stmt: trimmed) {
		parent->stmts.push_back(stmt);
	}
	for (auto stmt: stmts_after_body) {
		parent->stmts.push_back(stmt);
	}

	// If the body of the while loop only has a single if condition and
	// the else part of the condition is just a break, fuse the if with 
	// the loop

	if (to<stmt_block>(new_while->body)->stmts.size() == 1 && isa<if_stmt>(to<stmt_block>(new_while->body)->stmts[0])) {
		if_stmt::Ptr if_body = to<if_stmt>(to<stmt_block>(new_while->body)->stmts[0]);
		
		stmt::Ptr then_stmt = if_body->then_stmt;
		stmt::Ptr else_stmt = if_body->else_stmt;
	
		if (isa<stmt_block> (else_stmt) && to<stmt_block>(else_stmt)->stmts.size() == 1) {
			if (isa<break_stmt>(to<stmt_block>(else_stmt)->stmts[0])) {
				new_while->cond = if_body->cond;
				//new_while->body = std::make_shared<stmt_block>();
				new_while->body = then_stmt;
				return;
			}
		}
		if (isa<stmt_block> (then_stmt) && to<stmt_block> (then_stmt)->stmts.size() == 1) {
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
}
void last_jump_finder::visit(goto_stmt::Ptr a) {
	if (a->label1 == jump_label) {
		has_jump_to = true;	
	}
}
}

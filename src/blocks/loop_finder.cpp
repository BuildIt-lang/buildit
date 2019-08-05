#include "blocks/loop_finder.h"

namespace block {

void ensure_back_has_goto(stmt_block::Ptr a, label::Ptr label_detect) {
	if (a->stmts.size() == 0) {
		break_stmt::Ptr new_break = std::make_shared<break_stmt>();
		a->stmts.push_back(new_break);
		return;
	}
	stmt::Ptr last_stmt = a->stmts.back();
	if (isa<if_stmt>(last_stmt)) {
		ensure_back_has_goto(to<stmt_block>(to<if_stmt>(last_stmt)->then_stmt), label_detect);
		ensure_back_has_goto(to<stmt_block>(to<if_stmt>(last_stmt)->else_stmt), label_detect);
	} else if (isa<goto_stmt>(last_stmt) && to<goto_stmt>(last_stmt)->label1 == label_detect) {
		a->stmts.pop_back();
		
	} else if (isa<goto_stmt>(last_stmt) && to<goto_stmt>(last_stmt)->label1 != label_detect) {
	} else {
		break_stmt::Ptr new_break = std::make_shared<break_stmt>();
		a->stmts.push_back(new_break);

	}
	return;
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
	
	ensure_back_has_goto(to<stmt_block>(new_while->body), a->label1);
	
	parent->stmts = stmts_before;
	parent->stmts.push_back(new_while);
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

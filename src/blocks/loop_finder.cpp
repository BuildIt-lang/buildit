#include "blocks/loop_finder.h"
#include "blocks/parent_finder.h"

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

void loop_finder::visit(label_stmt::Ptr a) {
	parent_finder finder;
	finder.to_find = a;
	ast->accept(&finder);
	assert(finder.found_parent != nullptr);
	
	// First separate out the stmts before the loop begin
	std::vector<stmt::Ptr> stmts_before;
	std::vector<stmt::Ptr> stmts_in_body;
	std::vector<stmt::Ptr> stmts_after_body;

	stmt::Ptr last_stmt = nullptr;

	for (auto stmt: finder.found_parent->stmts) {
		last_jump_finder jump_finder;
		jump_finder.jump_label = a->label1;
		stmt->accept(&jump_finder);
		if (jump_finder.has_jump_to == true) 
			last_stmt = stmt;	
	}
	std::vector<stmt::Ptr>::iterator stmt;	
	for (stmt = finder.found_parent->stmts.begin(); stmt != finder.found_parent->stmts.end(); stmt++) {
		if (*stmt == a)
			break;
		stmts_before.push_back(*stmt);
	}
	stmt++;
	for (; stmt != finder.found_parent->stmts.end(); stmt++) {
		stmts_in_body.push_back(*stmt);
		if (*stmt == last_stmt)
			break;
	}
	stmt++;
	for (; stmt != finder.found_parent->stmts.end(); stmt++) {
		stmts_after_body.push_back(*stmt);
	}
	finder.found_parent->stmts.clear();

	while_stmt::Ptr new_while = std::make_shared<while_stmt>();
	new_while->cond = std::make_shared<int_const>();
	to<int_const>(new_while->cond)->value = 1;
	new_while->body = std::make_shared<stmt_block>();	
	to<stmt_block>(new_while->body)->stmts = stmts_in_body;
	
	ensure_back_has_goto(to<stmt_block>(new_while->body), a->label1);
	
	finder.found_parent->stmts = stmts_before;
	finder.found_parent->stmts.push_back(new_while);
	for (auto stmt: stmts_after_body) {
		finder.found_parent->stmts.push_back(stmt);
	}
}
void last_jump_finder::visit(goto_stmt::Ptr a) {
	if (a->label1 == jump_label) {
		has_jump_to = true;	
	}
}
}

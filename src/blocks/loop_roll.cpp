#include "blocks/loop_roll.h"
#include "builder/builder.h"
#include "builder/dyn_var.h"
namespace block {
static bool is_roll(std::string s) {
	if (s != "" && s.length() > 5 && s[0] == 'r' && s[1] == 'o' && s[2] == 'l' && s[3] == 'l' && s[4] == '.')
		return true;
	return false;
}
static int unique_counter = 0;
static void process_match(stmt_block::Ptr b, int match_start, int match_end) {
	stmt::Ptr first = b->stmts[match_start];
	constant_expr_finder finder;
	first->accept(&finder);
	unsigned int num_constants = finder.constants.size();
	// Assume int constants for now
	std::vector<std::vector<int>> vals;
	vals.resize(num_constants);
	for (unsigned int i = 0; i < num_constants; i++) {
		vals[i].push_back(to<int_const>(finder.constants[i])->value);
	}
	for (int i = match_start + 1; i < match_end; i++) {
		constant_expr_finder finder;
		b->stmts[i]->accept(&finder);
		assert(finder.constants.size() == num_constants);
		for (unsigned int j = 0; j < num_constants; j++)
			vals[j].push_back(to<int_const>(finder.constants[j])->value);
	}
	std::vector<stmt::Ptr> new_stmts;
	for (int i = 0; i < match_start; i++) {
		new_stmts.push_back(b->stmts[i]);
	}
	std::vector<var::Ptr> new_vars;
	for (unsigned int i = 0; i < num_constants; i++) {
		// Check a special case if const[index] == index, skip
		bool all_match = true;
		for (unsigned int j = 0; j < vals[i].size(); j++) {
			if (vals[i][j] != (int)j) {
				all_match = false;
				break;
			}
		}
		if (all_match) {
			new_vars.push_back(nullptr);
			continue;
		}
		var::Ptr new_var = std::make_shared<var>();
		new_var->var_name = "roll_var_" + std::to_string(unique_counter);

		std::vector<std::string> attrs;
		attrs.push_back("static");

		new_var->setMetadata("attributes", attrs);

		unique_counter++;
		new_var->var_type = builder::dyn_var<int[]>::create_block_type();
		to<array_type>(new_var->var_type)->size = match_end - match_start;
		decl_stmt::Ptr new_decl = std::make_shared<decl_stmt>();
		new_decl->decl_var = new_var;
		new_vars.push_back(new_var);

		initializer_list_expr::Ptr new_init = std::make_shared<initializer_list_expr>();
		for (unsigned int j = 0; j < vals[i].size(); j++) {
			int_const::Ptr new_const = std::make_shared<int_const>();
			new_const->value = vals[i][j];
			new_init->elems.push_back(new_const);
		}
		new_decl->init_expr = new_init;
		new_stmts.push_back(new_decl);
	}
	for_stmt::Ptr new_for = std::make_shared<for_stmt>();

	var::Ptr i_var = std::make_shared<var>();
	i_var->var_name = "index_var_" + std::to_string(unique_counter);
	unique_counter++;
	i_var->var_type = builder::dyn_var<int>::create_block_type();
	decl_stmt::Ptr new_decl = std::make_shared<decl_stmt>();
	new_decl->decl_var = i_var;
	int_const::Ptr init_const = std::make_shared<int_const>();
	init_const->value = 0;
	new_decl->init_expr = init_const;
	new_for->decl_stmt = new_decl;

	lt_expr::Ptr new_lt = std::make_shared<lt_expr>();
	var_expr::Ptr new_var_expr = std::make_shared<var_expr>();
	new_var_expr->var1 = i_var;
	int_const::Ptr new_const_expr = std::make_shared<int_const>();
	new_const_expr->value = match_end - match_start;
	new_lt->expr1 = new_var_expr;
	new_lt->expr2 = new_const_expr;
	new_for->cond = new_lt;

	assign_expr::Ptr new_assign = std::make_shared<assign_expr>();
	new_var_expr = std::make_shared<var_expr>();
	new_var_expr->var1 = i_var;

	var_expr::Ptr new_var_expr2 = std::make_shared<var_expr>();
	new_var_expr2->var1 = i_var;

	new_const_expr = std::make_shared<int_const>();
	new_const_expr->value = 1;

	plus_expr::Ptr new_plus = std::make_shared<plus_expr>();
	new_plus->expr1 = new_var_expr2;
	new_plus->expr2 = new_const_expr;

	new_assign->var1 = new_var_expr;
	new_assign->expr1 = new_plus;
	new_for->update = new_assign;

	new_for->body = std::make_shared<stmt_block>();

	// Replace constants
	std::vector<expr::Ptr> replace;

	for (unsigned int i = 0; i < num_constants; i++) {
		if (new_vars[i] == nullptr) {
			var_expr::Ptr new_var2 = std::make_shared<var_expr>();
			new_var2->var1 = i_var;
			replace.push_back(new_var2);
			continue;
		}

		sq_bkt_expr::Ptr new_sq_bkt = std::make_shared<sq_bkt_expr>();

		var_expr::Ptr new_var1 = std::make_shared<var_expr>();
		new_var1->var1 = new_vars[i];

		var_expr::Ptr new_var2 = std::make_shared<var_expr>();
		new_var2->var1 = i_var;

		new_sq_bkt->var_expr = new_var1;
		new_sq_bkt->index = new_var2;

		replace.push_back(new_sq_bkt);
	}

	constant_replacer replacer;
	replacer.replace = replace;

	first->accept(&replacer);

	to<stmt_block>(new_for->body)->stmts.push_back(first);
	first->annotation = "from." + first->annotation;

	new_stmts.push_back(new_for);

	for (unsigned int i = match_end; i < b->stmts.size(); i++)
		new_stmts.push_back(b->stmts[i]);
	b->stmts = new_stmts;
}
void loop_roll_finder::visit(stmt_block::Ptr b) {
	// First visit all the children
	for (auto stmt : b->stmts) {
		stmt->accept(this);
	}
	// Now find opportunities for rolling
	while (1) {
		int match_start = -1;
		int match_end = -1;
		for (unsigned int i = 0; i < b->stmts.size(); i++) {
			auto stmt = b->stmts[i];
			if (is_roll(stmt->annotation)) {
				std::string match = stmt->annotation;
				match_start = i;
				match_end = b->stmts.size();
				for (; i < b->stmts.size(); i++) {
					if (b->stmts[i]->annotation != match) {
						match_end = i;
						break;
					}
				}
			}
		}
		if (match_start != -1) {
			process_match(b, match_start, match_end);
		} else
			break;
	}
}
void constant_expr_finder::visit(int_const::Ptr a) {
	constants.push_back(a);
}

void constant_replacer::visit(int_const::Ptr a) {
	node = replace[curr_index++];
}
/*
void constant_replacer::visit(plus_expr::Ptr a) {
	if (isa<const_expr>(a->expr1)) {
		a->expr1 = replace[curr_index++];
	} else
		a->expr1->accept(this);
	if (isa<const_expr>(a->expr2)) {
		a->expr2 = replace[curr_index++];
	} else
		a->expr2->accept(this);
}
void constant_replacer::visit(mul_expr::Ptr a) {
	if (isa<const_expr>(a->expr1)) {
		a->expr1 = replace[curr_index++];
	} else
		a->expr1->accept(this);
	if (isa<const_expr>(a->expr2)) {
		a->expr2 = replace[curr_index++];
	} else
		a->expr2->accept(this);
}
void constant_replacer::visit(minus_expr::Ptr a) {
	if (isa<const_expr>(a->expr1)) {
		a->expr1 = replace[curr_index++];
	} else
		a->expr1->accept(this);
	if (isa<const_expr>(a->expr2)) {
		a->expr2 = replace[curr_index++];
	} else
		a->expr2->accept(this);
}
void constant_replacer::visit(div_expr::Ptr a) {
	if (isa<const_expr>(a->expr1)) {
		a->expr1 = replace[curr_index++];
	} else
		a->expr1->accept(this);
	if (isa<const_expr>(a->expr2)) {
		a->expr2 = replace[curr_index++];
	} else
		a->expr2->accept(this);
}
void constant_replacer::visit(sq_bkt_expr::Ptr a) {
	if (isa<const_expr>(a->var_expr)) {
		a->var_expr = replace[curr_index++];
	} else
		a->var_expr->accept(this);
	if (isa<const_expr>(a->index)) {
		a->index = replace[curr_index++];
	} else
		a->index->accept(this);
}
*/
} // namespace block

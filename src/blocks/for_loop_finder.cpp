#include "blocks/for_loop_finder.h"

namespace block {

static bool is_update_expr(var::Ptr decl_var, expr::Ptr last_stmt_expr) {

	if (isa<minus_expr>(last_stmt_expr)) {
		// This is to allow statements like (a = a + 1) - 1;
		minus_expr::Ptr mexpr = to<minus_expr>(last_stmt_expr);
		return is_update_expr(decl_var, mexpr->expr1);
	}

	if (!isa<assign_expr>(last_stmt_expr))
		return false;
	assign_expr::Ptr update_expr = to<assign_expr>(last_stmt_expr);
	if (!isa<var_expr>(update_expr->var1))
		return false;
	var_expr::Ptr lhs_expr = to<var_expr>(update_expr->var1);
	if (decl_var != lhs_expr->var1)
		return false;
	if (!isa<plus_expr>(update_expr->expr1))
		return false;
	plus_expr::Ptr rhs_expr = to<plus_expr>(update_expr->expr1);
	if (!isa<var_expr>(rhs_expr->expr1))
		return false;
	lhs_expr = to<var_expr>(rhs_expr->expr1);
	if (decl_var != lhs_expr->var1)
		return false;
	return true;
}

static bool is_update(var::Ptr decl_var, stmt::Ptr last_stmt) {
	if (!isa<expr_stmt>(last_stmt))
		return false;
	expr::Ptr last_stmt_expr = to<expr_stmt>(last_stmt)->expr1;

	return is_update_expr(decl_var, last_stmt_expr);
}
static bool is_last_update(var::Ptr decl_var, stmt_block::Ptr block, std::vector<stmt_block::Ptr> &parents) {
	if (block->stmts.size() == 0)
		return false;
	if (isa<break_stmt>(block->stmts.back()))
		return true;
	if (is_update(decl_var, block->stmts.back())) {
		parents.push_back(block);
		return true;
	}
	if (isa<if_stmt>(block->stmts.back())) {
		if_stmt::Ptr last_stmt = to<if_stmt>(block->stmts.back());
		if (!isa<stmt_block>(last_stmt->then_stmt))
			return false;
		if (!isa<stmt_block>(last_stmt->else_stmt))
			return false;
		bool then_res = is_last_update(decl_var, to<stmt_block>(last_stmt->then_stmt), parents);
		bool else_res = is_last_update(decl_var, to<stmt_block>(last_stmt->else_stmt), parents);
		if (then_res && else_res)
			return true;
		return false;
	}
	return false;
}

void var_use_finder::visit(var_expr::Ptr ve) {
	if (ve->var1 == to_find) {
		found = true;
	}
}

static bool has_further_uses(decl_stmt::Ptr decl, std::vector<stmt::Ptr> stmts, int start_index) {
	var_use_finder finder;
	finder.to_find = decl->decl_var;
	for (unsigned i = start_index; i < stmts.size(); i++) {
		stmts[i]->accept(&finder);
		if (finder.found)
			return true;
	}
	return false;
}
void for_loop_finder::visit(stmt_block::Ptr a) {
	while (1) {
		int while_loop_index = -1;
		std::vector<stmt_block::Ptr> parents;
		for (unsigned int i = 0; i < a->stmts.size(); i++) {
			parents.clear();
			if (isa<while_stmt>(a->stmts[i])) {
				while_stmt::Ptr loop = to<while_stmt>(a->stmts[i]);
				// All checks for while loop -> for loop
				// conversion
				if (i == 0)
					continue;
				if (!(isa<decl_stmt>(a->stmts[i - 1]) ||
				      (isa<expr_stmt>(a->stmts[i - 1]) &&
				       isa<assign_expr>(to<expr_stmt>(a->stmts[i - 1])->expr1))))
					continue;

				var::Ptr init_var;
				if (isa<decl_stmt>(a->stmts[i - 1])) {
					decl_stmt::Ptr decl = to<decl_stmt>(a->stmts[i - 1]);
					init_var = decl->decl_var;
				} else {
					auto assign = to<assign_expr>(to<expr_stmt>(a->stmts[i - 1])->expr1);
					if (!isa<var_expr>(assign->var1))
						continue;
					init_var = to<var_expr>(assign->var1)->var1;
				}

				if (!isa<lt_expr>(loop->cond))
					continue;
				lt_expr::Ptr cond_expr = to<lt_expr>(loop->cond);
				if (!isa<var_expr>(cond_expr->expr1))
					continue;
				var_expr::Ptr lhs_expr = to<var_expr>(cond_expr->expr1);

				if (init_var != lhs_expr->var1)
					continue;
				if (!isa<stmt_block>(loop->body))
					continue;

				stmt_block::Ptr loop_body = to<stmt_block>(loop->body);
				if (loop_body->stmts.size() < 1)
					continue;

				if (!is_last_update(init_var, loop_body, parents))
					continue;

				/*
								if (parents.size() == 0)
									continue;
				*/
				for (auto stmt : loop->continue_blocks) {
					if (loop->continue_blocks.size() < 2)
						continue;
					if (!is_update(init_var, stmt->stmts[stmt->stmts.size() - 2]))
						continue;
				}
				while_loop_index = i - 1;
				break;
			}
		}
		if (while_loop_index != -1) {
			while_stmt::Ptr loop = to<while_stmt>(a->stmts[while_loop_index + 1]);
			std::vector<stmt::Ptr> new_stmts;
			for (int i = 0; i < while_loop_index; i++)
				new_stmts.push_back(a->stmts[i]);
			for_stmt::Ptr for_loop = std::make_shared<for_stmt>();
			for_loop->static_offset = a->stmts[while_loop_index]->static_offset;
			// Before we merge the decl with the for loop, make sure
			// the variable being declared doesn't have any other uses
			auto decl = a->stmts[while_loop_index];
			if (isa<decl_stmt>(decl) &&
			    has_further_uses(to<decl_stmt>(decl), a->stmts, while_loop_index + 2)) {
				auto ce = std::make_shared<int_const>();
				ce->value = 0;
				auto es = std::make_shared<expr_stmt>();
				es->expr1 = ce;
				for_loop->decl_stmt = es;
				// Keep the decl in the new stmts
				new_stmts.push_back(decl);
			} else
				for_loop->decl_stmt = a->stmts[while_loop_index];
			for_loop->annotation = for_loop->decl_stmt->annotation;
			for_loop->decl_stmt->annotation = "";
			for_loop->cond = loop->cond;
			if (parents.size() == 0) {
				for_loop->update = nullptr;
			} else {
				for_loop->update = to<expr_stmt>(parents[0]->stmts.back())->expr1;
			}
			for (unsigned int i = 0; i < parents.size(); i++)
				parents[i]->stmts.pop_back();
			for (auto stmt : loop->continue_blocks) {
				auto cont_stmt = stmt->stmts.back();
				// This is the continue
				stmt->stmts.pop_back();
				// It is possible that none of the parents had an update
				// This could happen becuase of breaks
				if (for_loop->update == nullptr)
					for_loop->update = to<expr_stmt>(stmt->stmts.back())->expr1;
				stmt->stmts.pop_back();
				stmt->stmts.push_back(cont_stmt);
			}
			// Even after this if the update is empty, just put an empty expression there
			if (for_loop->update == nullptr) {
				auto ce = std::make_shared<int_const>();
				ce->value = 0;
				for_loop->update = ce;
			}

			for_loop->body = loop->body;
			new_stmts.push_back(for_loop);
			for (unsigned int i = while_loop_index + 2; i < a->stmts.size(); i++)
				new_stmts.push_back(a->stmts[i]);
			a->stmts = new_stmts;
		} else
			break;
	}
	for (unsigned int i = 0; i < a->stmts.size(); i++)
		a->stmts[i]->accept(this);
}
} // namespace block

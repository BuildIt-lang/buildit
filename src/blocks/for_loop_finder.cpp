#include "blocks/for_loop_finder.h"

namespace block {

void for_loop_finder::visit(stmt_block::Ptr a) {
	while (1) {
		int while_loop_index = -1;
		for (int i = 0; i < a->stmts.size(); i++) {
			if (isa<while_stmt>(a->stmts[i])) {
				while_stmt::Ptr loop = to<while_stmt>(a->stmts[i]);
				// All checks for while loop -> for loop conversion
				if (i == 0)
					continue;
				if (!isa<decl_stmt>(a->stmts[i-1]))
					continue;
				decl_stmt::Ptr decl = to<decl_stmt>(a->stmts[i-1]);
				var::Ptr decl_var = decl->decl_var;
				if (!isa<lt_expr>(loop->cond))
					continue;
				lt_expr::Ptr cond_expr = to<lt_expr>(loop->cond);
				if (!isa<var_expr>(cond_expr->expr1))
					continue;
				var_expr::Ptr lhs_expr = to<var_expr>(cond_expr->expr1);
				if (decl_var != lhs_expr->var1)
					continue;
				if (!isa<stmt_block>(loop->body))
					continue;
				stmt_block::Ptr loop_body = to<stmt_block>(loop->body);
				if (loop_body->stmts.size() < 1)
					continue;
				stmt::Ptr last_stmt = loop_body->stmts.back();
				if (!isa<expr_stmt>(last_stmt))
					continue;
				expr::Ptr last_stmt_expr = to<expr_stmt>(last_stmt)->expr1;
				if (!isa<assign_expr>(last_stmt_expr))
					continue;
				assign_expr::Ptr update_expr = to<assign_expr>(last_stmt_expr);
				if(!isa<var_expr>(update_expr->var1))
					continue;
				lhs_expr = to<var_expr>(update_expr->var1);
				if (decl_var != lhs_expr->var1)
					continue;
				if (!isa<plus_expr>(update_expr->expr1))
					continue;
				plus_expr::Ptr rhs_expr = to<plus_expr>(update_expr->expr1);
				if (!isa<var_expr>(rhs_expr->expr1))
					continue;
				lhs_expr = to<var_expr>(rhs_expr->expr1);
				if (decl_var != lhs_expr->var1)
					continue;		
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
			for_loop->decl_stmt = a->stmts[while_loop_index];
			for_loop->cond = loop->cond;
			for_loop->update = to<expr_stmt>(to<stmt_block>(loop->body)->stmts.back())->expr1;
			to<stmt_block>(loop->body)->stmts.pop_back();
			for_loop->body = loop->body;
			new_stmts.push_back(for_loop);
			for (int i = while_loop_index + 2; i < a->stmts.size(); i++)
				new_stmts.push_back(a->stmts[i]);
			a->stmts = new_stmts;
		} else
			break;
	}
	for (int i = 0; i < a->stmts.size(); i++)
		a->stmts[i]->accept(this);
}
}

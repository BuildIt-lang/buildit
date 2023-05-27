#include "blocks/rce.h"
#include "blocks/block_replacer.h"
#include "blocks/block_visitor.h"
#include "blocks/expr.h"
#include "blocks/stmt.h"
#include <algorithm>
#include <map>
#include <vector>
namespace block {

class var_use_counter : public block_visitor {
public:
	using block_visitor::visit;
	std::map<var::Ptr, uint64_t> usage_count;
	std::vector<var::Ptr> assigned_vars;
	virtual void visit(var_expr::Ptr e) override {
		var::Ptr v = e->var1;
		if (usage_count.find(v) != usage_count.end())
			usage_count[v]++;
		else
			usage_count[v] = 1;
	}
	virtual void visit(assign_expr::Ptr e) override {
		e->var1->accept(this);
		e->expr1->accept(this);
		if (!isa<var_expr>(e->var1))
			return;
		var_expr::Ptr ve = to<var_expr>(e->var1);
		var::Ptr v = ve->var1;
		if (std::find(assigned_vars.begin(), assigned_vars.end(), v) == assigned_vars.end())
			assigned_vars.push_back(v);
	}
};

class check_side_effects : public block_visitor {
public:
	using block_visitor::visit;
	bool has_side_effects = false;
	virtual void visit(assign_expr::Ptr) override {
		has_side_effects = true;
	}
	virtual void visit(function_call_expr::Ptr) override {
		has_side_effects = true;
	}
	virtual void visit(addr_of_expr::Ptr) override {
		has_side_effects = true;
	}
};

class gather_rce_decls : public block_visitor {
public:
	using block_visitor::visit;
	std::vector<decl_stmt::Ptr> gathered_decls;
	// This are vars (y) that may have one use but the use is of the form int x = y (use of y)
	// and x has multiple uses. When x is replaced with y it will have multiple uses
	// so we want to black list such x's
	std::vector<var::Ptr> duplicated_vars;
	std::map<var::Ptr, uint64_t> usage_count;
	std::vector<var::Ptr> assigned_vars;

	virtual void visit(decl_stmt::Ptr decl) {
		if (decl->init_expr == nullptr)
			return;
		var::Ptr v = decl->decl_var;
		if (std::find(assigned_vars.begin(), assigned_vars.end(), v) != assigned_vars.end())
			return;
		int use_count = 0;
		if (usage_count.find(v) != usage_count.end())
			use_count = usage_count[v];
		if (isa<var_expr>(decl->init_expr)) {
			// This is time to blacklist y
			if (use_count > 1) {
				var_expr::Ptr ve = to<var_expr>(decl->init_expr);
				var::Ptr v = ve->var1;
				if (std::find(duplicated_vars.begin(), duplicated_vars.end(), v) ==
				    duplicated_vars.end()) {
					duplicated_vars.push_back(v);
				}
			}
			gathered_decls.push_back(decl);
			return;
		}

		if (use_count == 1) {
			check_side_effects checker;
			decl->init_expr->accept(&checker);
			// We will also allow variables that are used exactly once
			// and don't have side effects
			if (checker.has_side_effects == false) {
				gathered_decls.push_back(decl);
				return;
			}
		}
	}
};

class replace_rce_vars : public block_replacer {
public:
	using block_replacer::visit;
	std::vector<decl_stmt::Ptr> gathered_decls;
	std::map<var::Ptr, decl_stmt::Ptr> var_decl_map;
	std::vector<var::Ptr> perma_enabled_decls;
	std::vector<var::Ptr> enabled_decls;

	virtual void visit(decl_stmt::Ptr decl) override {
		if (decl->init_expr)
			decl->init_expr = rewrite(decl->init_expr);
		if (std::find(gathered_decls.begin(), gathered_decls.end(), decl) != gathered_decls.end()) {
			if (isa<var_expr>(decl->init_expr))
				perma_enabled_decls.push_back(decl->decl_var);
			else {
				// Store decls that have a complex expression on the RHS
				// separately, if there is any statement that has side-effects,
				// we can immediately clear this
				enabled_decls.push_back(decl->decl_var);
			}
		}
		node = decl;
	}
	virtual void visit(assign_expr::Ptr assign) override {
		assign->expr1 = rewrite(assign->expr1);
		assign->var1 = rewrite(assign->var1);
		enabled_decls.clear();
		node = assign;
	}
	virtual void visit(addr_of_expr::Ptr addr) override {
		addr->expr1 = rewrite(addr->expr1);
		enabled_decls.clear();
		node = addr;
	}
	virtual void visit(function_call_expr::Ptr f) override {
		for (unsigned int i = 0; i < f->args.size(); i++) {
			f->args[i] = rewrite(f->args[i]);
		}
		enabled_decls.clear();
		node = f;
	}
	virtual void visit(var_expr::Ptr ve) override {
		var::Ptr v = ve->var1;
		if (std::find(perma_enabled_decls.begin(), perma_enabled_decls.end(), v) != perma_enabled_decls.end() ||
		    std::find(enabled_decls.begin(), enabled_decls.end(), v) != enabled_decls.end()) {
			decl_stmt::Ptr de = var_decl_map[v];
			node = de->init_expr;
		} else {
			node = ve;
		}
	}
};

class rce_decl_deleter : public block_visitor {
public:
	using block_visitor::visit;
	std::map<var::Ptr, uint64_t> usage_count;
	virtual void visit(stmt_block::Ptr b) {
		std::vector<stmt::Ptr> new_stmts;
		for (auto stmt : b->stmts) {
			if (isa<decl_stmt>(stmt)) {
				var::Ptr v = to<decl_stmt>(stmt)->decl_var;
				if (usage_count.find(v) != usage_count.end()) {
					// TODO: ensure that we are not deleting decls who's RHS have side-effects
					new_stmts.push_back(stmt);
				}
			} else {
				new_stmts.push_back(stmt);
			}
			stmt->accept(this);
		}
		b->stmts = new_stmts;
	}
};

void eliminate_redundant_vars(block::Ptr ast) {
	var_use_counter counter;
	ast->accept(&counter);
	gather_rce_decls gatherer;
	gatherer.usage_count = counter.usage_count;
	gatherer.assigned_vars = counter.assigned_vars;
	ast->accept(&gatherer);

	replace_rce_vars replacer;
	for (auto decl : gatherer.gathered_decls) {
		var::Ptr v = decl->decl_var;

		if (!isa<var_expr>(decl->init_expr) &&
		    std::find(gatherer.duplicated_vars.begin(), gatherer.duplicated_vars.end(), v) !=
			gatherer.duplicated_vars.end())
			continue;
		replacer.gathered_decls.push_back(decl);
		replacer.var_decl_map[v] = decl;
	}
	ast->accept(&replacer);
	// Now that all th replacements have been done, we will decls that are not used
	var_use_counter post_counter;
	ast->accept(&post_counter);

	rce_decl_deleter deleter;
	deleter.usage_count = post_counter.usage_count;
	ast->accept(&deleter);
}

} // namespace block

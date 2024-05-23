#include "blocks/rce.h"
#include "blocks/block_replacer.h"
#include "blocks/block_visitor.h"
#include "blocks/expr.h"
#include "blocks/stmt.h"
#include <map>
#include <algorithm>

namespace block {

// General utility visitor to find usage statistics
class usage_counter: public block_visitor {
public:
	using block_visitor::visit;

	std::map<var::Ptr, int> usage_count;
	std::vector<var::Ptr> address_taken_vars;

	virtual void visit(var_expr::Ptr e) override {	
		var::Ptr v = e->var1;
		if (usage_count.find(v) != usage_count.end())
			usage_count[v]++;
		else
			usage_count[v] = 1;
	}	

	// We don't care if variables are assigned to, but only ones that
	// have their address taken. 
	// TODO: also discard variables that have references bound to them
	virtual void visit(addr_of_expr::Ptr e) override {
		e->expr1->accept(this);
		if (!isa<var_expr>(e->expr1))
			return;
		var::Ptr v = to<var_expr>(e->expr1)->var1;
		if (std::find(address_taken_vars.begin(), address_taken_vars.end(), v) == address_taken_vars.end())
			address_taken_vars.push_back(v);	
	}
};
// General utility to check if an expression has side effects 
class check_side_effects: public block_visitor {
public:
	using block_visitor::visit;

	bool has_side_effects = false;
	virtual void visit(assign_expr::Ptr) override {
		has_side_effects = true;
	}

	virtual void visit(function_call_expr::Ptr) override {
		has_side_effects = true;	
	}
};

static bool has_side_effects(block::Ptr b) {
	check_side_effects checker;
	b->accept(&checker);
	return checker.has_side_effects;	
}
	

// Both RCE phases leave variables that have their addresses taken, untouched

// Phase 1 finds vars of the form int x = <complex expression> where x only has 
// one use and <complex expression> has no side effects. 
// With this, the substitution is stopped as soon as any side effect occurs, like assignment or function calls

class phase1_visitor: public block_replacer {
public:
	using block_replacer::visit;

	std::map<var::Ptr, expr::Ptr> value_map;
	std::map<var::Ptr, int> usage_count;
	std::vector<var::Ptr> address_taken_vars;
	
	virtual void visit(decl_stmt::Ptr ds) override {
		// We are not changing decls, so this is okay to be written first
		node = ds;

		// Before we do anything, visit the RHS
		// If there is no RHS, stop
		if (ds->init_expr != nullptr) {
			if (has_side_effects(ds->init_expr))
				value_map.clear();
			ds->init_expr = rewrite(ds->init_expr);
		} else 
			return;

		// Check if this variable is eligible for phase 1 RCE
		var::Ptr v = ds->decl_var;
		// If the variable has no usage info, stop
		if (usage_count.find(v) == usage_count.end())
			return;
		// If usage count > 1 stop
		if (usage_count[v] > 1) 
			return;
		// If variable has its address taken stop
		if (std::find(address_taken_vars.begin(), address_taken_vars.end(), v) != address_taken_vars.end())
			return;
		// Finally check if the init expr as side effects
		if (has_side_effects(ds->init_expr))
			return;
		
		// All good, we are ready to substiture this variable
		value_map[v] = ds->init_expr;
	}

	virtual void visit(expr_stmt::Ptr es) override {
		node = es;

		// TODO: Special case for top-level assign exprs to defer clearing if each 
		// subexpression has no side effects

		if (has_side_effects(es->expr1))
			value_map.clear();
		es->expr1 = rewrite(es->expr1);	
	}

	virtual void visit(return_stmt::Ptr rs) override {
		node = rs;
		if (has_side_effects(rs->return_val))
			value_map.clear();
		rs->return_val = rewrite(rs->return_val);
	}
	
	virtual void visit(while_stmt::Ptr ws) override {
		node = ws;
		if (has_side_effects(ws->cond))
			value_map.clear();
		ws->cond = rewrite(ws->cond);
		ws->body = rewrite<stmt>(ws->body);
	}
	virtual void visit(for_stmt::Ptr fs) override {
		node = fs;
		fs->decl_stmt = rewrite<stmt>(fs->decl_stmt);
		if (has_side_effects(fs->cond))
			value_map.clear();
		if (has_side_effects(fs->update))
			value_map.clear();
		fs->cond = rewrite(fs->cond);
		fs->update = rewrite(fs->update);
		fs->body = rewrite<stmt>(fs->body);
	}
	virtual void visit(if_stmt::Ptr is) override {
		node = is;
		if (has_side_effects(is->cond)) 
			value_map.clear();
		is->cond = rewrite(is->cond);
		is->then_stmt = rewrite<stmt>(is->then_stmt);
		is->else_stmt = rewrite<stmt>(is->else_stmt);
	}


	virtual void visit(var_expr::Ptr ve) override {
		node = ve;
		if (value_map.find(ve->var1) == value_map.end())
			return;	
		// If we have a substitution make it now
		node = value_map[ve->var1];
	}
};

class decl_deleter: public block_visitor {
public:
	using block_visitor::visit;
	std::map<var::Ptr, int> usage_count;
	virtual void visit(stmt_block::Ptr sb) override {
		std::vector<stmt::Ptr> new_stmts;
		for (auto stmt : sb->stmts) {
			if (!isa<decl_stmt>(stmt)) {
				stmt->accept(this);
				new_stmts.push_back(stmt);
				continue;
			}
			decl_stmt::Ptr ds = to<decl_stmt>(stmt);
			var::Ptr dv = ds->decl_var;
			if (usage_count.find(dv) != usage_count.end() && usage_count[dv] > 0) {
				// No need to visit decl stmts, they cannot have decl stmts inside (right?)
				new_stmts.push_back(stmt);
				continue;
			}
			if (ds->init_expr != nullptr) {
				check_side_effects checker;
				ds->init_expr->accept(&checker);
				if (checker.has_side_effects) {
					new_stmts.push_back(stmt);
					continue;
				}
			}
			// All good, we are ready to drop
		}
		sb->stmts = new_stmts;
	}
};

static void rce_phase1(block::Ptr ast) {
	// gather general statistics first
	usage_counter counter;
	ast->accept(&counter);


	phase1_visitor p1v;
	p1v.usage_count = counter.usage_count;
	p1v.address_taken_vars = counter.address_taken_vars;
	ast->accept(&p1v);
	
	// Perform a second usage count before cleanup
	usage_counter counter2;
	ast->accept(&counter2);

	decl_deleter deleter;
	deleter.usage_count = counter2.usage_count;
	ast->accept(&deleter);
	
	// Phase 1 RCE done	
	
}

static void rce_phase2(block::Ptr ast) {
}

void eliminate_redundant_vars(block::Ptr ast) {
	rce_phase1(ast);	
	rce_phase2(ast);
}

}

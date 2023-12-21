#include "blocks/var_namer.h"
#include <algorithm>
namespace block {

void var_gather_escapes::visit(decl_stmt::Ptr stmt) {
	if (stmt->decl_var->hasMetadata<int>("escapes_static_scope") &&
	    stmt->decl_var->getMetadata<int>("escapes_static_scope")) {

		if (!stmt->decl_var->hasMetadata<int>("allow_escape_scope") ||
		    !stmt->decl_var->getMetadata<int>("allow_escape_scope")) {

			std::string so_loc = stmt->decl_var->static_offset.stringify_loc();
			if (std::find(escaping_tags.begin(), escaping_tags.end(), so_loc) == escaping_tags.end())
				escaping_tags.push_back(so_loc);
		}
	}
}

static std::string get_apt_tag(var::Ptr a, std::vector<std::string> escaping_tags) {
	std::string so_loc = a->static_offset.stringify_loc();
	if (std::find(escaping_tags.begin(), escaping_tags.end(), so_loc) != escaping_tags.end())
		return so_loc;
	else
		return a->static_offset.stringify();
}

void var_namer::visit(decl_stmt::Ptr stmt) {
	std::string so = get_apt_tag(stmt->decl_var, escaping_tags);

	if (collected_decls.find(so) != collected_decls.end()) {
		// This decl has been seen before, and needs to be marked for hoisting
		decls_to_hoist[so] = stmt;
		if (std::find(decl_tags_to_hoist.begin(), decl_tags_to_hoist.end(), so) == decl_tags_to_hoist.end())
			decl_tags_to_hoist.push_back(so);
		// Also make the replacement now
		stmt->decl_var = collected_decls[so];
		return;
	}

	// If a variable already has a name, created with with_name(_, true), skip naming it
	if (stmt->decl_var->var_name != "")
		return;

	// We have found a new variable decl, first assign it a name
	if (stmt->decl_var->preferred_name != "") {
		stmt->decl_var->var_name = stmt->decl_var->preferred_name + "_" + std::to_string(var_counter);
	} else {
		stmt->decl_var->var_name = "var" + std::to_string(var_counter);
	}
	var_counter++;
	// Now record this decl to be used else where
	collected_decls[so] = stmt->decl_var;
}

void var_replacer::visit(var_expr::Ptr a) {
	std::string so = get_apt_tag(a->var1, escaping_tags);

	if (collected_decls.find(so) != collected_decls.end()) {
		a->var1 = collected_decls[so];
	}
}

void var_hoister::visit(decl_stmt::Ptr a) {
	std::string so = get_apt_tag(a->decl_var, escaping_tags);
	if (decls_to_hoist.find(so) != decls_to_hoist.end()) {
		// This decl needs to be flattened into an assignment
		// but if it doesn't have an init_expr, just make a simple var_expr
		expr_stmt::Ptr estmt = std::make_shared<expr_stmt>();
		estmt->static_offset = a->static_offset;
		estmt->annotation = a->annotation;

		var_expr::Ptr vexpr = std::make_shared<var_expr>();
		vexpr->static_offset = a->static_offset;
		vexpr->var1 = a->decl_var;

		if (a->init_expr == nullptr) {
			estmt->expr1 = vexpr;
			node = estmt;
			return;
		} else {
			assign_expr::Ptr assign = std::make_shared<assign_expr>();
			assign->static_offset = a->static_offset;

			assign->var1 = vexpr;
			assign->expr1 = a->init_expr;

			estmt->expr1 = assign;
		}
		node = estmt;
		return;
	}
	node = a;
}

void var_namer::name_vars(block::Ptr a) {
	var_namer namer;

	var_gather_escapes gatherer(namer.escaping_tags);
	a->accept(&gatherer);

	a->accept(&namer);

	var_replacer replacer(namer.collected_decls, namer.escaping_tags);
	a->accept(&replacer);

	var_hoister hoister(namer.decls_to_hoist, namer.escaping_tags);
	a->accept(&hoister);

	std::vector<stmt::Ptr> new_stmts;
	// Now insert all the hoisted decls at the top
	for (auto dt : namer.decl_tags_to_hoist) {
		auto stmt = namer.decls_to_hoist[dt];
		stmt->init_expr = nullptr;
		new_stmts.push_back(stmt);
	}
	stmt_block::Ptr b = to<stmt_block>(a);
	for (auto stmt : b->stmts) {
		new_stmts.push_back(stmt);
	}
	b->stmts = new_stmts;
}

} // namespace block

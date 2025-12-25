#include "builder/dyn_var.h"
#include "builder/generics.h"
namespace builder {

std::vector<dyn_var_base *> *parents_stack = nullptr;
bool user_defined_provider_track_members = false;

int allocatable_type_registry::type_counter = 0;
std::vector<allocatable_type_registry::deleter_t>* allocatable_type_registry::type_deleters = nullptr;

void dyn_var_base::set_type(type t) {
	assert(var_mode == standalone_var);
	block_var->var_type = t.enclosed_type;
}

block::expr::Ptr to_expr(const dyn_var_base& d) {
	if (d.var_mode == dyn_var_base::standalone_var) {
		auto e = create_expr<block::var_expr>({});
		e->var1 = d.block_var;
		return e;
	} else if (d.var_mode == dyn_var_base::member_var) {
		auto parent_expr = to_expr(*(d.parent_var));
		auto e = create_expr<block::member_access_expr>({parent_expr});
		e->parent_expr = parent_expr;
		e->member_name = d.member_name;
		return e;	
	} else if (d.var_mode == dyn_var_base::compound_expr) {
		return d.block_expr;
	} else {
		assert(false && "Bad mode for variable");
	}
	return nullptr;
}
block::expr::Ptr to_expr(const float& v) {
	auto e = create_expr<block::float_const>({});
	e->value = v;
	return e;
}

block::expr::Ptr to_expr(const double& v) {
	auto e = create_expr<block::double_const>({});
	e->value = v;
	return e;
}
block::expr::Ptr to_expr(const std::string& s) {
	auto e = create_expr<block::string_const>({});
	e->value = s;
	return e;
}
block::expr::Ptr to_expr(const char* s) {
	return to_expr((std::string)s);
}
block::expr::Ptr to_expr(char* s) {
	return to_expr((std::string)s);
}
block::expr::Ptr to_expr(const std::initializer_list<expr_wrapper>& i) {
	std::vector<expr_wrapper> args(i);
	std::vector<block::expr::Ptr> argv;
	for (auto x: args) {
		argv.push_back(x.e);
	}	
	auto e = create_expr<block::initializer_list_expr>(argv);
	e->elems = argv;
	return e;
}
block::expr::Ptr to_expr(const expr_wrapper_base& e) {
	return e.e;
}

}

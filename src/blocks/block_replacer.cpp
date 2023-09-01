#include "blocks/block_replacer.h"
#include "blocks/stmt.h"

namespace block {
void block_replacer::visit(block::Ptr a) {
	node = a;
}
void block_replacer::visit(expr::Ptr a) {
	node = a;
}

void block_replacer::unary_helper(unary_expr::Ptr a) {
	a->expr1 = rewrite(a->expr1);
	node = a;
}
void block_replacer::binary_helper(binary_expr::Ptr a) {
	a->expr1 = rewrite(a->expr1);
	a->expr2 = rewrite(a->expr2);
	node = a;
}
void block_replacer::visit(unary_expr::Ptr a) {
	unary_helper(a);
}
void block_replacer::visit(binary_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(not_expr::Ptr a) {
	unary_helper(a);
}
void block_replacer::visit(and_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(bitwise_and_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(or_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(bitwise_or_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(plus_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(minus_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(mul_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(div_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(lt_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(gt_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(lte_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(gte_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(lshift_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(rshift_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(equals_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(ne_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(mod_expr::Ptr a) {
	binary_helper(a);
}
void block_replacer::visit(var_expr::Ptr a) {
	a->var1 = rewrite<var>(a->var1);
	node = a;
}
void block_replacer::visit(const_expr::Ptr a) {
	node = a;
}
void block_replacer::visit(int_const::Ptr a) {
	node = a;
}
void block_replacer::visit(double_const::Ptr a) {
	node = a;
}
void block_replacer::visit(float_const::Ptr a) {
	node = a;
}
void block_replacer::visit(string_const::Ptr a) {
	node = a;
}
void block_replacer::visit(assign_expr::Ptr a) {
	a->var1 = rewrite(a->var1);
	a->expr1 = rewrite(a->expr1);
	node = a;
}
void block_replacer::visit(stmt::Ptr a) {
	node = a;
}
void block_replacer::visit(expr_stmt::Ptr a) {
	a->expr1 = rewrite(a->expr1);
	node = a;
}
void block_replacer::visit(stmt_block::Ptr a) {
	for (unsigned int i = 0; i < a->stmts.size(); i++) {
		auto tmp = rewrite<stmt>(a->stmts[i]);
		a->stmts.at(i) = tmp;
	}
	node = a;
}
void block_replacer::visit(decl_stmt::Ptr a) {
	a->decl_var = rewrite<var>(a->decl_var);
	if (a->init_expr != nullptr)
		a->init_expr = rewrite(a->init_expr);
	node = a;
}
void block_replacer::visit(if_stmt::Ptr a) {
	a->cond = rewrite(a->cond);
	a->then_stmt = rewrite<stmt>(a->then_stmt);
	a->else_stmt = rewrite<stmt>(a->else_stmt);
	node = a;
}
void block_replacer::visit(label::Ptr a) {
	node = a;
}
void block_replacer::visit(label_stmt::Ptr a) {
	if (a->label1 != nullptr)
		a->label1 = rewrite<label>(a->label1);
	node = a;
}
void block_replacer::visit(goto_stmt::Ptr a) {
	if (a->label1 != nullptr)
		a->label1 = rewrite<label>(a->label1);
	node = a;
}
void block_replacer::visit(break_stmt::Ptr a) {
	node = a;
}
void block_replacer::visit(continue_stmt::Ptr a) {
	node = a;
}
void block_replacer::visit(while_stmt::Ptr a) {
	a->cond = rewrite(a->cond);
	a->body = rewrite<stmt>(a->body);
	node = a;
}
void block_replacer::visit(for_stmt::Ptr a) {
	a->decl_stmt = rewrite<stmt>(a->decl_stmt);
	a->cond = rewrite(a->cond);
	a->update = rewrite(a->update);
	a->body = rewrite<stmt>(a->body);
	node = a;
}
void block_replacer::visit(sq_bkt_expr::Ptr a) {
	a->var_expr = rewrite(a->var_expr);
	a->index = rewrite(a->index);
	node = a;
}
void block_replacer::visit(function_call_expr::Ptr a) {
	a->expr1 = rewrite(a->expr1);

	for (unsigned int i = 0; i < a->args.size(); i++) {
		auto tmp = rewrite(a->args[i]);
		a->args.at(i) = tmp;
	}
	node = a;
}
void block_replacer::visit(initializer_list_expr::Ptr a) {

	for (unsigned int i = 0; i < a->elems.size(); i++) {
		auto tmp = rewrite(a->elems[i]);
		a->elems.at(i) = tmp;
	}
	node = a;
}
void block_replacer::visit(foreign_expr_base::Ptr a) {
	// Since this is an abstract class, we do nothing
	node = a;
}
void block_replacer::visit(var::Ptr a) {
	a->var_type = rewrite<type>(a->var_type);
	node = a;
}
void block_replacer::visit(type::Ptr a) {
	node = a;
}
void block_replacer::visit(scalar_type::Ptr a) {
	node = a;
}
void block_replacer::visit(pointer_type::Ptr a) {
	a->pointee_type = rewrite<type>(a->pointee_type);
	node = a;
}
void block_replacer::visit(reference_type::Ptr a) {
	a->referenced_type = rewrite<type>(a->referenced_type);
	node = a;
}
void block_replacer::visit(function_type::Ptr a) {
	a->return_type = rewrite<type>(a->return_type);
	for (unsigned int i = 0; i < a->arg_types.size(); i++) {
		auto tmp = rewrite<type>(a->arg_types[i]);
		a->arg_types.at(i) = tmp;
	}
	node = a;
}
void block_replacer::visit(array_type::Ptr a) {
	a->element_type = rewrite<type>(a->element_type);
	node = a;
}
void block_replacer::visit(builder_var_type::Ptr a) {
	a->closure_type = rewrite<type>(a->closure_type);
	node = a;
}
void block_replacer::visit(named_type::Ptr a) {
	std::vector<type::Ptr> new_args;
	for (auto b : a->template_args) {
		new_args.push_back(rewrite<type>(b));
	}
	a->template_args = new_args;
	node = a;
}
void block_replacer::visit(func_decl::Ptr a) {
	a->return_type = rewrite<type>(a->return_type);
	for (unsigned int i = 0; i < a->args.size(); i++) {
		auto tmp = rewrite<var>(a->args[i]);
		a->args.at(i) = tmp;
	}
	a->body = rewrite<stmt>(a->body);
	node = a;
}
void block_replacer::visit(return_stmt::Ptr a) {
	a->return_val = rewrite(a->return_val);
	node = a;
}

void block_replacer::visit(member_access_expr::Ptr a) {
	a->parent_expr = rewrite(a->parent_expr);
	node = a;
}
void block_replacer::visit(addr_of_expr::Ptr a) {
	a->expr1 = rewrite(a->expr1);
	node = a;
}

} // namespace block

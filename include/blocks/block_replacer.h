#ifndef BLOCK_REPLACER_H
#define BLOCK_REPLACER_H

#include "blocks/block.h"
#include "blocks/block_visitor.h"
namespace block {
class block_replacer : public block_visitor {
public:
	using block_visitor::visit;
	std::shared_ptr<block> node;
	template <typename T = expr>
	typename T::Ptr rewrite(typename T::Ptr ptr) {
		auto tmp = node;
		node = nullptr;
		ptr->accept(this);
		auto ret = to<T>(node);
		node = tmp;
		return ret;
	}
	void unary_helper(std::shared_ptr<unary_expr> a);
	void binary_helper(std::shared_ptr<binary_expr> a);

	virtual void visit(std::shared_ptr<block>) override;
	virtual void visit(std::shared_ptr<expr>) override;
	virtual void visit(std::shared_ptr<unary_expr>) override;
	virtual void visit(std::shared_ptr<binary_expr>) override;
	virtual void visit(std::shared_ptr<not_expr>) override;
	virtual void visit(std::shared_ptr<and_expr>) override;
	virtual void visit(std::shared_ptr<bitwise_and_expr>) override;
	virtual void visit(std::shared_ptr<or_expr>) override;
	virtual void visit(std::shared_ptr<bitwise_or_expr>) override;
	virtual void visit(std::shared_ptr<plus_expr>) override;
	virtual void visit(std::shared_ptr<minus_expr>) override;
	virtual void visit(std::shared_ptr<mul_expr>) override;
	virtual void visit(std::shared_ptr<div_expr>) override;
	virtual void visit(std::shared_ptr<lt_expr>) override;
	virtual void visit(std::shared_ptr<gt_expr>) override;
	virtual void visit(std::shared_ptr<lte_expr>) override;
	virtual void visit(std::shared_ptr<gte_expr>) override;
	virtual void visit(std::shared_ptr<lshift_expr>) override;
	virtual void visit(std::shared_ptr<rshift_expr>) override;
	virtual void visit(std::shared_ptr<equals_expr>) override;
	virtual void visit(std::shared_ptr<ne_expr>) override;
	virtual void visit(std::shared_ptr<mod_expr>) override;
	virtual void visit(std::shared_ptr<var_expr>) override;
	virtual void visit(std::shared_ptr<const_expr>) override;
	virtual void visit(std::shared_ptr<int_const>) override;
	virtual void visit(std::shared_ptr<double_const>) override;
	virtual void visit(std::shared_ptr<float_const>) override;
	virtual void visit(std::shared_ptr<string_const>) override;
	virtual void visit(std::shared_ptr<assign_expr>) override;
	virtual void visit(std::shared_ptr<stmt>) override;
	virtual void visit(std::shared_ptr<expr_stmt>) override;
	virtual void visit(std::shared_ptr<stmt_block>) override;
	virtual void visit(std::shared_ptr<decl_stmt>) override;
	virtual void visit(std::shared_ptr<if_stmt>) override;
	virtual void visit(std::shared_ptr<label>) override;
	virtual void visit(std::shared_ptr<label_stmt>) override;
	virtual void visit(std::shared_ptr<goto_stmt>) override;
	virtual void visit(std::shared_ptr<while_stmt>) override;
	virtual void visit(std::shared_ptr<for_stmt>) override;
	virtual void visit(std::shared_ptr<break_stmt>) override;
	virtual void visit(std::shared_ptr<continue_stmt>) override;
	virtual void visit(std::shared_ptr<sq_bkt_expr>) override;
	virtual void visit(std::shared_ptr<function_call_expr>) override;
	virtual void visit(std::shared_ptr<initializer_list_expr>) override;

	virtual void visit(std::shared_ptr<foreign_expr_base>) override;
	virtual void visit(std::shared_ptr<member_access_expr>) override;
	virtual void visit(std::shared_ptr<addr_of_expr>) override;

	virtual void visit(std::shared_ptr<var>) override;
	virtual void visit(std::shared_ptr<type>) override;
	virtual void visit(std::shared_ptr<scalar_type>) override;
	virtual void visit(std::shared_ptr<pointer_type>) override;
	virtual void visit(std::shared_ptr<reference_type>) override;
	virtual void visit(std::shared_ptr<function_type>) override;
	virtual void visit(std::shared_ptr<array_type>) override;
	virtual void visit(std::shared_ptr<builder_var_type>) override;
	virtual void visit(std::shared_ptr<named_type>) override;

	virtual void visit(std::shared_ptr<func_decl>) override;
	virtual void visit(std::shared_ptr<return_stmt>) override;
};
} // namespace block

#endif

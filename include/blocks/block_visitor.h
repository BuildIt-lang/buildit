#ifndef BLOCK_VISITOR_H
#define BLOCK_VISITOR_H
#include <memory>

namespace block {
class block;
class expr;
class unary_expr;
class binary_expr;
class not_expr;
class and_expr;
class bitwise_and_expr;
class or_expr;
class bitwise_or_expr;
class plus_expr;
class minus_expr;
class mul_expr;
class div_expr;
class lt_expr;
class gt_expr;
class lte_expr;
class gte_expr;
class lshift_expr;
class rshift_expr;
class equals_expr;
class ne_expr;
class mod_expr;
class var_expr;
class const_expr;
class int_const;
class double_const;
class float_const;
class string_const;
class assign_expr;
class stmt;
class expr_stmt;
class stmt_block;
class decl_stmt;
class if_stmt;
class label;
class label_stmt;
class goto_stmt;
class while_stmt;
class for_stmt;
class break_stmt;
class continue_stmt;
class sq_bkt_expr;
class function_call_expr;
class initializer_list_expr;

class foreign_expr_base;
class member_access_expr;
class addr_of_expr;

class var;
class type;
class scalar_type;
class pointer_type;
class reference_type;
class function_type;
class array_type;
class builder_var_type;
class named_type;

class func_decl;
class return_stmt;

class block_visitor {
public:
	virtual void visit(std::shared_ptr<block>);
	virtual void visit(std::shared_ptr<expr>);
	virtual void visit(std::shared_ptr<unary_expr>);
	virtual void visit(std::shared_ptr<binary_expr>);
	virtual void visit(std::shared_ptr<not_expr>);
	virtual void visit(std::shared_ptr<and_expr>);
	virtual void visit(std::shared_ptr<bitwise_and_expr>);
	virtual void visit(std::shared_ptr<or_expr>);
	virtual void visit(std::shared_ptr<bitwise_or_expr>);
	virtual void visit(std::shared_ptr<plus_expr>);
	virtual void visit(std::shared_ptr<minus_expr>);
	virtual void visit(std::shared_ptr<mul_expr>);
	virtual void visit(std::shared_ptr<div_expr>);
	virtual void visit(std::shared_ptr<lt_expr>);
	virtual void visit(std::shared_ptr<gt_expr>);
	virtual void visit(std::shared_ptr<lte_expr>);
	virtual void visit(std::shared_ptr<gte_expr>);
	virtual void visit(std::shared_ptr<lshift_expr>);
	virtual void visit(std::shared_ptr<rshift_expr>);
	virtual void visit(std::shared_ptr<equals_expr>);
	virtual void visit(std::shared_ptr<ne_expr>);
	virtual void visit(std::shared_ptr<mod_expr>);
	virtual void visit(std::shared_ptr<var_expr>);
	virtual void visit(std::shared_ptr<const_expr>);
	virtual void visit(std::shared_ptr<int_const>);
	virtual void visit(std::shared_ptr<double_const>);
	virtual void visit(std::shared_ptr<float_const>);
	virtual void visit(std::shared_ptr<string_const>);
	virtual void visit(std::shared_ptr<assign_expr>);
	virtual void visit(std::shared_ptr<stmt>);
	virtual void visit(std::shared_ptr<expr_stmt>);
	virtual void visit(std::shared_ptr<stmt_block>);
	virtual void visit(std::shared_ptr<decl_stmt>);
	virtual void visit(std::shared_ptr<if_stmt>);
	virtual void visit(std::shared_ptr<label>);
	virtual void visit(std::shared_ptr<label_stmt>);
	virtual void visit(std::shared_ptr<goto_stmt>);
	virtual void visit(std::shared_ptr<while_stmt>);
	virtual void visit(std::shared_ptr<for_stmt>);
	virtual void visit(std::shared_ptr<break_stmt>);
	virtual void visit(std::shared_ptr<continue_stmt>);
	virtual void visit(std::shared_ptr<sq_bkt_expr>);
	virtual void visit(std::shared_ptr<function_call_expr>);
	virtual void visit(std::shared_ptr<initializer_list_expr>);

	virtual void visit(std::shared_ptr<foreign_expr_base>);
	virtual void visit(std::shared_ptr<member_access_expr>);
	virtual void visit(std::shared_ptr<addr_of_expr>);

	virtual void visit(std::shared_ptr<var>);
	virtual void visit(std::shared_ptr<type>);
	virtual void visit(std::shared_ptr<scalar_type>);
	virtual void visit(std::shared_ptr<pointer_type>);
	virtual void visit(std::shared_ptr<reference_type>);
	virtual void visit(std::shared_ptr<function_type>);
	virtual void visit(std::shared_ptr<array_type>);
	virtual void visit(std::shared_ptr<builder_var_type>);
	virtual void visit(std::shared_ptr<named_type>);

	virtual void visit(std::shared_ptr<func_decl>);
	virtual void visit(std::shared_ptr<return_stmt>);
};
} // namespace block

#endif

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
class or_expr;
class plus_expr;
class minus_expr;
class mul_expr;
class div_expr;
class lt_expr;
class gt_expr;
class lte_expr;
class gte_expr;
class equals_expr;
class ne_expr;
class var_expr;
class const_expr;
class int_const;
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
class break_stmt;
class sq_bkt_expr;


class var;
class type;
class scalar_type;
class pointer_type;

class block_visitor {
public:
	virtual void visit (std::shared_ptr<block>);
	virtual void visit (std::shared_ptr<expr>);
	virtual void visit (std::shared_ptr<unary_expr>);
	virtual void visit (std::shared_ptr<binary_expr>);
	virtual void visit (std::shared_ptr<not_expr>);
	virtual void visit (std::shared_ptr<and_expr>);
	virtual void visit (std::shared_ptr<or_expr>);
	virtual void visit (std::shared_ptr<plus_expr>);
	virtual void visit (std::shared_ptr<minus_expr>);
	virtual void visit (std::shared_ptr<mul_expr>);
	virtual void visit (std::shared_ptr<div_expr>);
	virtual void visit (std::shared_ptr<lt_expr>);
	virtual void visit (std::shared_ptr<gt_expr>);
	virtual void visit (std::shared_ptr<lte_expr>);
	virtual void visit (std::shared_ptr<gte_expr>);
	virtual void visit (std::shared_ptr<equals_expr>);
	virtual void visit (std::shared_ptr<ne_expr>);
	virtual void visit (std::shared_ptr<var_expr>);
	virtual void visit (std::shared_ptr<const_expr>);
	virtual void visit (std::shared_ptr<int_const>);
	virtual void visit (std::shared_ptr<assign_expr>);
	virtual void visit (std::shared_ptr<stmt>);
	virtual void visit (std::shared_ptr<expr_stmt>);
	virtual void visit (std::shared_ptr<stmt_block>);
	virtual void visit (std::shared_ptr<decl_stmt>);
	virtual void visit (std::shared_ptr<if_stmt>);
	virtual void visit (std::shared_ptr<label>);
	virtual void visit (std::shared_ptr<label_stmt>);
	virtual void visit (std::shared_ptr<goto_stmt>);	
	virtual void visit (std::shared_ptr<while_stmt>);	
	virtual void visit (std::shared_ptr<break_stmt>);	
	virtual void visit (std::shared_ptr<sq_bkt_expr>);

	virtual void visit (std::shared_ptr<var>);
	virtual void visit (std::shared_ptr<type>);
	virtual void visit (std::shared_ptr<scalar_type>);
	virtual void visit (std::shared_ptr<pointer_type>);

};
}

#endif
#ifndef STMT_H
#define STMT_H
#include "blocks/expr.h"
#include <vector>
namespace block{
class stmt: public block {
public:
	typedef std::shared_ptr<stmt> Ptr;
	virtual void dump(std::ostream&, int) override;
	
};

class expr_stmt: public stmt {
public:
	typedef std::shared_ptr<expr_stmt> Ptr;
	virtual void dump(std::ostream&, int) override;
	
	expr::Ptr expr1;
};

class stmt_block: public stmt {
public:
	typedef std::shared_ptr<stmt_block> Ptr;
	virtual void dump(std::ostream&, int) override;

	std::vector<stmt::Ptr> stmts;	
};

class decl_stmt: public stmt {
public:
	typedef std::shared_ptr<decl_stmt> Ptr;
	virtual void dump(std::ostream&, int);
	
	var::Ptr decl_var;
	// Optional initialization
	expr::Ptr init_expr = nullptr;
};
}
#endif

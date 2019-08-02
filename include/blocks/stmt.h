#ifndef STMT_H
#define STMT_H
#include "blocks/expr.h"
#include <vector>
namespace block{
class stmt: public block {
public:
	typedef std::shared_ptr<stmt> Ptr;
	virtual void dump(std::ostream&, int) override;
	virtual void accept(block_visitor *a) {
		a->visit(self<stmt>());
	}
	
};

class expr_stmt: public stmt {
public:
	typedef std::shared_ptr<expr_stmt> Ptr;
	virtual void dump(std::ostream&, int) override;
	virtual void accept(block_visitor *a) {
		a->visit(self<expr_stmt>());
	}
	
	expr::Ptr expr1;
};

class stmt_block: public stmt {
public:
	typedef std::shared_ptr<stmt_block> Ptr;
	virtual void dump(std::ostream&, int) override;
	virtual void accept(block_visitor *a) {
		a->visit(self<stmt_block>());
	}

	std::vector<stmt::Ptr> stmts;	
};

class decl_stmt: public stmt {
public:
	typedef std::shared_ptr<decl_stmt> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor *a) {
		a->visit(self<decl_stmt>());
	}
	
	var::Ptr decl_var;
	// Optional initialization
	expr::Ptr init_expr = nullptr;
};

class if_stmt: public stmt {
public:
	typedef std::shared_ptr<if_stmt> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor *a) {
		a->visit(self<if_stmt>());
	}
	
	expr::Ptr cond;
	stmt::Ptr then_stmt;
	stmt::Ptr else_stmt;

};
class label: public block{
public:
	typedef std::shared_ptr<label> Ptr;
	virtual void dump(std::ostream&, int);
	std::string label_name;
	virtual void accept(block_visitor *a) {
		a->visit(self<label>());
	}

};
class label_stmt: public stmt {
public:
	typedef std::shared_ptr<label_stmt> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor *a) {
		a->visit(self<label_stmt>());
	}
	
	label::Ptr label1;
};
class goto_stmt: public stmt {
public:
	typedef std::shared_ptr<goto_stmt> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor *a) {
		a->visit(self<goto_stmt>());
	}
	
	label::Ptr label1;	
	int32_t temporary_label_number;
};
class while_stmt: public stmt {
public:
	typedef std::shared_ptr<while_stmt> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor *a) {
		a->visit(self<while_stmt>());
	}
	stmt::Ptr body;
};
class break_stmt: public stmt {
public:
	typedef std::shared_ptr<break_stmt> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor *a) {
		a->visit(self<break_stmt>());
	}
};
}
#endif

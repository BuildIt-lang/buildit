#ifndef EXPR_H
#define EXPR_H

#include "blocks/block.h"
#include "blocks/var.h"

#include <string>
#include <memory>
namespace block{
class expr: public block {
public:
	typedef std::shared_ptr<expr> Ptr; 		
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<expr>());
	}
};
class unary_expr: public expr {
public:
	typedef std::shared_ptr<unary_expr> Ptr;
	expr::Ptr expr1;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<unary_expr>());
	}
	
};

class binary_expr: public expr {
public:
	typedef std::shared_ptr<binary_expr> Ptr;
	
	virtual void dump(std::ostream&, int);
	expr::Ptr expr1;
	expr::Ptr expr2;
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<binary_expr>());
	}

};

// For the logical not operator
class not_expr: public unary_expr {
public:
	typedef std::shared_ptr<not_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<not_expr>());
	}
};


class and_expr: public binary_expr {
public: 
	typedef std::shared_ptr<and_expr> Ptr;	
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<and_expr>());
	}
};


class or_expr: public binary_expr {
public:
	typedef std::shared_ptr<or_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<or_expr>());
	}
};


class plus_expr: public binary_expr {
public: 
	typedef std::shared_ptr<plus_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<plus_expr>());
	}
};


class minus_expr: public binary_expr {
public:
	typedef std::shared_ptr<minus_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<minus_expr>());
	}
};


class mul_expr: public binary_expr {
public: 
	typedef std::shared_ptr<mul_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<mul_expr>());
	}
};


class div_expr: public binary_expr {
public: 
	typedef std::shared_ptr<div_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<div_expr>());
	}
};
class lt_expr: public binary_expr {
public:
	typedef std::shared_ptr<lt_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<lt_expr>());
	}
};
class gt_expr: public binary_expr {
public:
	typedef std::shared_ptr<gt_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<gt_expr>());
	}
};
class lte_expr: public binary_expr {
public:
	typedef std::shared_ptr<lte_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<lte_expr>());
	}
};
class gte_expr: public binary_expr {
public:
	typedef std::shared_ptr<gte_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<gte_expr>());
	}
};

class equals_expr: public binary_expr {
public:
	typedef std::shared_ptr<equals_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<equals_expr>());
	}
};

class ne_expr: public binary_expr {
public:
	typedef std::shared_ptr<ne_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<ne_expr>());
	}
};

class var_expr: public expr {
public:
	typedef std::shared_ptr<var_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<var_expr>());
	}

	var::Ptr var1;	
};

class const_expr: public expr {
public:
	typedef std::shared_ptr<const_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<const_expr>());
	}
};

class int_const: public const_expr {
public:
	typedef std::shared_ptr<int_const> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<int_const>());
	}
	
	long long value;
};

class assign_expr: public expr {
public:
	typedef std::shared_ptr<assign_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<assign_expr>());
	}
	
	expr::Ptr var1;	
	expr::Ptr expr1;

};
class sq_bkt_expr: public expr {
public:
	typedef std::shared_ptr<sq_bkt_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
	}
	expr::Ptr var_expr;
	expr::Ptr index;
};
}

#endif

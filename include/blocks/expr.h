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
	virtual bool is_same(block::Ptr other);
};
template <typename T>
bool unary_is_same (std::shared_ptr<T> first, block::Ptr other);

template <typename T>
bool binary_is_same (std::shared_ptr<T> first, block::Ptr other);

class unary_expr: public expr {
public:
	typedef std::shared_ptr<unary_expr> Ptr;
	expr::Ptr expr1;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<unary_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return unary_is_same(self<unary_expr>(), other);
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
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<binary_expr>(), other);
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
	virtual bool is_same(block::Ptr other) {
		return unary_is_same(self<not_expr>(), other);
	}
	
};


class and_expr: public binary_expr {
public: 
	typedef std::shared_ptr<and_expr> Ptr;	
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<and_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<and_expr>(), other);
	}
};


class or_expr: public binary_expr {
public:
	typedef std::shared_ptr<or_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<or_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<or_expr>(), other);
	}
};


class plus_expr: public binary_expr {
public: 
	typedef std::shared_ptr<plus_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<plus_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<plus_expr>(), other);
	}
};


class minus_expr: public binary_expr {
public:
	typedef std::shared_ptr<minus_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<minus_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<minus_expr>(), other);
	}
};


class mul_expr: public binary_expr {
public: 
	typedef std::shared_ptr<mul_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<mul_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<mul_expr>(), other);
	}
};


class div_expr: public binary_expr {
public: 
	typedef std::shared_ptr<div_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<div_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<div_expr>(), other);
	}
};
class lt_expr: public binary_expr {
public:
	typedef std::shared_ptr<lt_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<lt_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<lt_expr>(), other);
	}
};
class gt_expr: public binary_expr {
public:
	typedef std::shared_ptr<gt_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<gt_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<gt_expr>(), other);
	}
};
class lte_expr: public binary_expr {
public:
	typedef std::shared_ptr<lte_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<lte_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<lte_expr>(), other);
	}
};
class gte_expr: public binary_expr {
public:
	typedef std::shared_ptr<gte_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<gte_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<gte_expr>(), other);
	}
};

class equals_expr: public binary_expr {
public:
	typedef std::shared_ptr<equals_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<equals_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<equals_expr>(), other);
	}
};

class ne_expr: public binary_expr {
public:
	typedef std::shared_ptr<ne_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<ne_expr>());
	}
	virtual bool is_same(block::Ptr other) {
		return binary_is_same(self<ne_expr>(), other);
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
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<var_expr>(other))
			return false;
		var_expr::Ptr other_expr = to<var_expr>(other);
		if (!var1->is_same(other_expr->var1))
			return false;
		return true;
	}
};

class const_expr: public expr {
public:
	typedef std::shared_ptr<const_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<const_expr>());
	}

	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<const_expr>(other))
			return false;
		return true;
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
		
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<int_const>(other))
			return false;
		int_const::Ptr other_expr = to<int_const>(other);
		if (value != other_expr->value)
			return false;
		return true;
	}
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
	
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset) 
			return false;
		if (!isa<assign_expr>(other))
			return false;
		assign_expr::Ptr other_expr = to<assign_expr>(other);
		if (!var1->is_same(other_expr->var1))
			return false;
		if (!expr1->is_same(other_expr->expr1))
			return false;
		return true;
	}

};
class sq_bkt_expr: public expr {
public:
	typedef std::shared_ptr<sq_bkt_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<sq_bkt_expr>());
	}
	expr::Ptr var_expr;
	expr::Ptr index;
	
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset) 
			return false;
		if (!isa<sq_bkt_expr>(other))
			return false;
		sq_bkt_expr::Ptr other_expr = to<sq_bkt_expr>(other);
		if (!var_expr->is_same(other_expr->var_expr))
			return false;
		if (!index->is_same(other_expr->index))
			return false;
		return true;
	}
};
class function_call_expr: public expr {
public:
	typedef std::shared_ptr<function_call_expr> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<function_call_expr>());
	}	
	
	expr::Ptr expr1;
	std::vector<expr::Ptr> args;
	
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset) 
			return false;
		if (!isa<function_call_expr>(other))
			return false;
		function_call_expr::Ptr other_expr = to<function_call_expr>(other);
		if (!expr1->is_same(other_expr->expr1))
			return false;
		if (args.size() != other_expr->args.size())
			return false;
		for (int i = 0; i < args.size(); i++) {
			if (!args[i]->is_same(other_expr->args[i]))
				return false;
		}
		return true;
	}
};
}

#endif

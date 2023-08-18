#ifndef EXPR_H
#define EXPR_H

#include "blocks/block.h"
#include "blocks/var.h"

#include <memory>
#include <string>
namespace block {
class expr : public block {
public:
	typedef std::shared_ptr<expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<expr>());
	}
	virtual bool is_same(block::Ptr other) override;
};
template <typename T>
bool unary_is_same(std::shared_ptr<T> first, block::Ptr other);

template <typename T>
bool binary_is_same(std::shared_ptr<T> first, block::Ptr other);

template <typename T>
bool unary_is_same(std::shared_ptr<T> first, block::Ptr other) {
	if (!isa<unary_expr>(other))
		return false;
	typename T::Ptr other_expr = to<T>(other);
	if (!(first->expr1->is_same(other_expr->expr1)))
		return false;
	return true;
}
template <typename T>
bool binary_is_same(std::shared_ptr<T> first, block::Ptr second) {
	if (!isa<T>(second))
		return false;
	typename T::Ptr other_expr = to<T>(second);
	if (!(first->expr1->is_same(other_expr->expr1)))
		return false;
	if (!(first->expr2->is_same(other_expr->expr2)))
		return false;
	return true;
}

class unary_expr : public expr {
public:
	typedef std::shared_ptr<unary_expr> Ptr;
	expr::Ptr expr1;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<unary_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return unary_is_same(self<unary_expr>(), other);
	}
};

class binary_expr : public expr {
public:
	typedef std::shared_ptr<binary_expr> Ptr;

	virtual void dump(std::ostream &, int) override;
	expr::Ptr expr1;
	expr::Ptr expr2;
	virtual void accept(block_visitor *a) override {
		a->visit(self<binary_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<binary_expr>(), other);
	}
};
// For the logical not operator
class not_expr : public unary_expr {
public:
	typedef std::shared_ptr<not_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<not_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return unary_is_same(self<not_expr>(), other);
	}
};

class and_expr : public binary_expr {
public:
	typedef std::shared_ptr<and_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<and_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<and_expr>(), other);
	}
};

class bitwise_and_expr : public binary_expr {
public:
	typedef std::shared_ptr<bitwise_and_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<bitwise_and_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<bitwise_and_expr>(), other);
	}
};

class or_expr : public binary_expr {
public:
	typedef std::shared_ptr<or_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<or_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<or_expr>(), other);
	}
};

class bitwise_or_expr : public binary_expr {
public:
	typedef std::shared_ptr<bitwise_or_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<bitwise_or_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<bitwise_or_expr>(), other);
	}
};

class plus_expr : public binary_expr {
public:
	typedef std::shared_ptr<plus_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<plus_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<plus_expr>(), other);
	}
};

class minus_expr : public binary_expr {
public:
	typedef std::shared_ptr<minus_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<minus_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<minus_expr>(), other);
	}
};

class mul_expr : public binary_expr {
public:
	typedef std::shared_ptr<mul_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<mul_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<mul_expr>(), other);
	}
};

class div_expr : public binary_expr {
public:
	typedef std::shared_ptr<div_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<div_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<div_expr>(), other);
	}
};
class lt_expr : public binary_expr {
public:
	typedef std::shared_ptr<lt_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<lt_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<lt_expr>(), other);
	}
};
class gt_expr : public binary_expr {
public:
	typedef std::shared_ptr<gt_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<gt_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<gt_expr>(), other);
	}
};
class lte_expr : public binary_expr {
public:
	typedef std::shared_ptr<lte_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<lte_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<lte_expr>(), other);
	}
};
class gte_expr : public binary_expr {
public:
	typedef std::shared_ptr<gte_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<gte_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<gte_expr>(), other);
	}
};
class lshift_expr : public binary_expr {
public:
	typedef std::shared_ptr<lshift_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<lshift_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<lshift_expr>(), other);
	}
};
class rshift_expr : public binary_expr {
public:
	typedef std::shared_ptr<rshift_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<rshift_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<rshift_expr>(), other);
	}
};
class equals_expr : public binary_expr {
public:
	typedef std::shared_ptr<equals_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<equals_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<equals_expr>(), other);
	}
};

class ne_expr : public binary_expr {
public:
	typedef std::shared_ptr<ne_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<ne_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<ne_expr>(), other);
	}
};

class mod_expr : public binary_expr {
public:
	typedef std::shared_ptr<mod_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<mod_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		return binary_is_same(self<mod_expr>(), other);
	}
};
class var_expr : public expr {
public:
	typedef std::shared_ptr<var_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<var_expr>());
	}

	var::Ptr var1;
	virtual bool is_same(block::Ptr other) override {
		if (!isa<var_expr>(other))
			return false;
		var_expr::Ptr other_expr = to<var_expr>(other);
		if (!var1->is_same(other_expr->var1))
			return false;
		return true;
	}
};

class const_expr : public expr {
public:
	typedef std::shared_ptr<const_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<const_expr>());
	}

	virtual bool is_same(block::Ptr other) override {
		if (!isa<const_expr>(other))
			return false;
		return true;
	}
};

class int_const : public const_expr {
public:
	typedef std::shared_ptr<int_const> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<int_const>());
	}

	long long value;
	bool is_64bit;

	virtual bool is_same(block::Ptr other) override {
		if (!isa<int_const>(other))
			return false;
		int_const::Ptr other_expr = to<int_const>(other);
		if (value != other_expr->value)
			return false;
		return true;
	}
};
class double_const : public const_expr {
public:
	typedef std::shared_ptr<double_const> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<double_const>());
	}

	double value;

	virtual bool is_same(block::Ptr other) override {
		if (!isa<double_const>(other))
			return false;
		double_const::Ptr other_expr = to<double_const>(other);
		if (value != other_expr->value)
			return false;
		return true;
	}
};
class float_const : public const_expr {
public:
	typedef std::shared_ptr<float_const> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<float_const>());
	}

	float value;

	virtual bool is_same(block::Ptr other) override {
		if (!isa<float_const>(other))
			return false;
		float_const::Ptr other_expr = to<float_const>(other);
		if (value != other_expr->value)
			return false;
		return true;
	}
};

class string_const : public const_expr {
public:
	typedef std::shared_ptr<string_const> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<string_const>());
	}

	std::string value;

	virtual bool is_same(block::Ptr other) override {
		if (!isa<string_const>(other))
			return false;
		string_const::Ptr other_expr = to<string_const>(other);
		if (value != other_expr->value)
			return false;
		return true;
	}
};

class assign_expr : public expr {
public:
	typedef std::shared_ptr<assign_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<assign_expr>());
	}

	expr::Ptr var1;
	expr::Ptr expr1;

	virtual bool is_same(block::Ptr other) override {
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
class sq_bkt_expr : public expr {
public:
	typedef std::shared_ptr<sq_bkt_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<sq_bkt_expr>());
	}
	expr::Ptr var_expr;
	expr::Ptr index;

	virtual bool is_same(block::Ptr other) override {
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
class function_call_expr : public expr {
public:
	typedef std::shared_ptr<function_call_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<function_call_expr>());
	}

	expr::Ptr expr1;
	std::vector<expr::Ptr> args;

	virtual bool is_same(block::Ptr other) override {
		if (!isa<function_call_expr>(other))
			return false;
		function_call_expr::Ptr other_expr = to<function_call_expr>(other);
		if (!expr1->is_same(other_expr->expr1))
			return false;
		if (args.size() != other_expr->args.size())
			return false;
		for (unsigned int i = 0; i < args.size(); i++) {
			if (!args[i]->is_same(other_expr->args[i]))
				return false;
		}
		return true;
	}
};

class initializer_list_expr : public expr {
public:
	typedef std::shared_ptr<initializer_list_expr> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<initializer_list_expr>());
	}

	std::vector<expr::Ptr> elems;
	virtual bool is_same(block::Ptr other) override {
		if (!isa<initializer_list_expr>(other))
			return false;
		initializer_list_expr::Ptr other_expr = to<initializer_list_expr>(other);
		if (elems.size() != other_expr->elems.size())
			return false;
		for (unsigned int i = 0; i < elems.size(); i++) {
			if (!elems[i]->is_same(other_expr->elems[i]))
				return false;
		}
		return true;
	}
};

// We first implement a base class for foreign_expr for visitor purposes
class foreign_expr_base : public expr {
public:
	typedef std::shared_ptr<foreign_expr_base> Ptr;
	// We do not need any other functions because this class is guaranteed
	// to abstract
protected:
	// We will add a protected constructor to make sure that this is truly
	// abstract
	foreign_expr_base() = default;
};
template <typename T>
class foreign_expr : public foreign_expr_base {
public:
	typedef std::shared_ptr<foreign_expr> Ptr;
	virtual void dump(std::ostream &oss, int i) override {
		printer::indent(oss, i);
		oss << "FOREIGN_EXPR" << std::endl;
	}
	virtual void accept(block_visitor *a) override {
		// We will call the visit function for the base class
		// because we cannot write visitor for template class
		// The base class visit function then has to check using isa<>
		a->visit(self<foreign_expr_base>());
	}

	T inner_expr;
	virtual bool is_same(block::Ptr other) override {
		if (!isa<foreign_expr<T>>(other))
			return false;
		foreign_expr<T>::Ptr other_expr = to<foreign_expr>(other);
		// We assume that T has == operator and that is the best we can
		// check
		if (!(inner_expr == other_expr->inner_expr))
			return false;
		return true;
	}
};

class member_access_expr : public expr {
public:
	expr::Ptr parent_expr;
	std::string member_name;

	typedef std::shared_ptr<member_access_expr> Ptr;
	virtual void dump(std::ostream &oss, int i) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<member_access_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		if (!isa<member_access_expr>(other))
			return false;
		member_access_expr::Ptr other_expr = to<member_access_expr>(other);
		if (!other_expr->parent_expr->is_same(parent_expr))
			return false;
		if (other_expr->member_name != member_name)
			return false;
		return true;
	}
};

class addr_of_expr : public expr {
public:
	expr::Ptr expr1;

	typedef std::shared_ptr<addr_of_expr> Ptr;
	virtual void dump(std::ostream &oss, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<addr_of_expr>());
	}
	virtual bool is_same(block::Ptr other) override {
		if (!isa<addr_of_expr>(other))
			return false;
		addr_of_expr::Ptr other_expr = to<addr_of_expr>(other);
		if (!other_expr->expr1->is_same(expr1))
			return false;
		return true;
	}
};
} // namespace block

#endif

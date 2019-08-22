#ifndef STMT_H
#define STMT_H
#include "blocks/expr.h"
#include <vector>
namespace block{
class stmt: public block {
public:
	typedef std::shared_ptr<stmt> Ptr;
	std::string annotation;
	virtual void dump(std::ostream&, int) override;
	virtual void accept(block_visitor *a) {
		a->visit(self<stmt>());
	}

	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<stmt>(other))
			return false;
		return true;
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
	
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<expr_stmt>(other))
			return false;
		expr_stmt::Ptr other_stmt = to<expr_stmt>(other);
		if (!expr1->is_same(other_stmt->expr1))
			return false;
		return true;
	}
};

class stmt_block: public stmt {
public:
	typedef std::shared_ptr<stmt_block> Ptr;
	virtual void dump(std::ostream&, int) override;
	virtual void accept(block_visitor *a) {
		a->visit(self<stmt_block>());
	}

	std::vector<stmt::Ptr> stmts;	
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<stmt_block>(other))
			return false;
		stmt_block::Ptr other_stmt = to<stmt_block>(other);
		if (stmts.size() != other_stmt->stmts.size())
			return false;
		
		for (int i = 0; i < stmts.size(); i++) {
			if (!stmts[i]->is_same(other_stmt->stmts[i]))
				return false;
		}
		return true;
	}
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
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<decl_stmt>(other))
			return false;
		decl_stmt::Ptr other_stmt = to<decl_stmt>(other);
		if (!decl_var->is_same(other_stmt->decl_var))
			return false;
		if (init_expr == nullptr && other_stmt->init_expr != nullptr)
			return false;
		if (init_expr != nullptr && other_stmt->init_expr == nullptr)
			return false;
		if (init_expr == nullptr && other_stmt->init_expr == nullptr)
			return true;
		if (!init_expr->is_same(other_stmt->init_expr))
			return false;
		return true;
	}
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
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<if_stmt>(other))
			return false;
		if_stmt::Ptr other_stmt = to<if_stmt>(other);
		if (!cond->is_same(other_stmt->cond))
			return false;
		if (!then_stmt->is_same(other_stmt->then_stmt))
			return false;
		if (!else_stmt->is_same(other_stmt->else_stmt))
			return false;	
	}

};
class label: public block {
public:
	typedef std::shared_ptr<label> Ptr;
	virtual void dump(std::ostream&, int);
	std::string label_name;
	virtual void accept(block_visitor *a) {
		a->visit(self<label>());
	}
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<label>(other))
			return false;
		label::Ptr other_stmt = to<label>(other);
		if (label_name != other_stmt->label_name)
			return false;
		return true;
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
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<label_stmt>(other))
			return false;
		label_stmt::Ptr other_stmt = to<label_stmt>(other);
		if (!label1->is_same(other_stmt->label1))
			return false;
		return true;
	}
};
class goto_stmt: public stmt {
public:
	typedef std::shared_ptr<goto_stmt> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor *a) {
		a->visit(self<goto_stmt>());
	}
	
	label::Ptr label1;	
	tracer::tag temporary_label_number;

	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<goto_stmt>(other))
			return false;
		goto_stmt::Ptr other_stmt = to<goto_stmt>(other);
		if (label1 == nullptr) {
			if (other_stmt->label1 != nullptr)
				return false;
			if (temporary_label_number != temporary_label_number)
				return false;
		} else {
			if (!label1->is_same(other_stmt->label1))
				return false;
		}
		return true;
	}
	
};
class while_stmt: public stmt {
public:
	typedef std::shared_ptr<while_stmt> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor *a) {
		a->visit(self<while_stmt>());
	}
	stmt::Ptr body;
	expr::Ptr cond;
	virtual bool is_same(block::Ptr other) {
		if (static_offset != other->static_offset)
			return false;
		if (!isa<while_stmt>(other))
			return false;
		while_stmt::Ptr other_stmt = to<while_stmt>(other);
		if (!body->is_same(other_stmt->body))
			return false;
		if (!cond->is_same(other_stmt->cond))
			return false;
		return true;
	}
};
class break_stmt: public stmt {
public:
	typedef std::shared_ptr<break_stmt> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor *a) {
		a->visit(self<break_stmt>());
	}
	virtual bool is_same(block::Ptr other) {
		if (!isa<break_stmt>(other))
			return false;
		return true;
	}
};
}
#endif

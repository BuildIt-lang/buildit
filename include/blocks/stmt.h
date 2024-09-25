#ifndef STMT_H
#define STMT_H
#include "blocks/expr.h"
#include <vector>
namespace block {
class stmt : public block {
public:
	typedef std::shared_ptr<stmt> Ptr;
	std::string annotation;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<stmt>());
	}

	virtual bool is_same(block::Ptr other) override {
		if (!isa<stmt>(other))
			return false;
		return true;
	}
};

template <typename T>
std::shared_ptr<T> clone_stmt(T* t) {
	auto np = clone_obj(t);
	np->annotation = t->annotation;
	return np;
}

class expr_stmt : public stmt {
public:
	typedef std::shared_ptr<expr_stmt> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<expr_stmt>());
	}

	expr::Ptr expr1;

	// member to keep track if this expr stmt
	// has been spuriously created and needs to be deleted
	bool mark_for_deletion = false;

	virtual bool is_same(block::Ptr other) override {
		if (!isa<expr_stmt>(other))
			return false;
		expr_stmt::Ptr other_stmt = to<expr_stmt>(other);
		if (!expr1->is_same(other_stmt->expr1))
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		np->expr1 = clone(expr1);
		np->mark_for_deletion = mark_for_deletion;
		return np;
	}
	
};

class stmt_block : public stmt {
public:
	typedef std::shared_ptr<stmt_block> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<stmt_block>());
	}

	std::vector<stmt::Ptr> stmts;
	virtual bool is_same(block::Ptr other) override {
		if (!isa<stmt_block>(other))
			return false;
		stmt_block::Ptr other_stmt = to<stmt_block>(other);
		if (stmts.size() != other_stmt->stmts.size())
			return false;

		for (unsigned int i = 0; i < stmts.size(); i++) {
			if (!stmts[i]->is_same(other_stmt->stmts[i]))
				return false;
		}
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		for (auto stmt: stmts) {
			np->stmts.push_back(clone(stmt));
		}
		return np;
	}
};

class decl_stmt : public stmt {
public:
	typedef std::shared_ptr<decl_stmt> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<decl_stmt>());
	}

	var::Ptr decl_var;
	// Optional initialization
	expr::Ptr init_expr = nullptr;
	virtual bool is_same(block::Ptr other) override {
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
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		// Vars are not to be cloned because 
		// they are compared by pointers sometimes

		// If we _do_ end up duplicating them, 
		// they should be consistently updated
		np->decl_var = decl_var;
		np->init_expr = clone(init_expr);
		return np;
	}
};

class if_stmt : public stmt {
public:
	typedef std::shared_ptr<if_stmt> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<if_stmt>());
	}

	expr::Ptr cond;
	stmt::Ptr then_stmt;
	stmt::Ptr else_stmt;
	virtual bool is_same(block::Ptr other) override {
		if (!isa<if_stmt>(other))
			return false;
		if_stmt::Ptr other_stmt = to<if_stmt>(other);
		if (!cond->is_same(other_stmt->cond))
			return false;
		if (!then_stmt->is_same(other_stmt->then_stmt))
			return false;
		if (!else_stmt->is_same(other_stmt->else_stmt))
			return false;
		return true;
	}
	virtual bool needs_splitting(if_stmt::Ptr other) {
		if (is_same(other))
			return false;
		if (static_offset != other->static_offset)
			return false;
		if (!then_stmt->is_same(other->then_stmt))
			return false;
		if (!else_stmt->is_same(other->else_stmt))
			return false;
		if (!cond->is_same(other->cond))
			return true;
		assert(false && "Statement unreachable");
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		np->cond = clone(cond);
		np->then_stmt = clone(then_stmt);
		np->else_stmt = clone(else_stmt);
		return np;
	}
};
class label : public block {
public:
	typedef std::shared_ptr<label> Ptr;
	virtual void dump(std::ostream &, int) override;
	std::string label_name;
	virtual void accept(block_visitor *a) override {
		a->visit(self<label>());
	}
	virtual bool is_same(block::Ptr other) override {
		if (!isa<label>(other))
			return false;
		label::Ptr other_stmt = to<label>(other);
		if (label_name != other_stmt->label_name)
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		// Don't duplicate labels
		// just like vars these might be compared by ptr
		return self<label>();
	}
};
class label_stmt : public stmt {
public:
	typedef std::shared_ptr<label_stmt> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<label_stmt>());
	}

	label::Ptr label1;
	virtual bool is_same(block::Ptr other) override {
		if (!isa<label_stmt>(other))
			return false;
		label_stmt::Ptr other_stmt = to<label_stmt>(other);
		if (!label1->is_same(other_stmt->label1))
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		// Doesn't need to be duplicated
		np->label1 = label1;
		return np;
	}
};
class goto_stmt : public stmt {
public:
	typedef std::shared_ptr<goto_stmt> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<goto_stmt>());
	}

	label::Ptr label1;
	tracer::tag temporary_label_number;

	virtual bool is_same(block::Ptr other) override {
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
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		np->label1 = label1;
		np->temporary_label_number = temporary_label_number;
		return np;
	}
};
class while_stmt : public stmt {
public:
	typedef std::shared_ptr<while_stmt> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<while_stmt>());
	}
	stmt::Ptr body;
	expr::Ptr cond;

	// Extra metadata
	std::vector<stmt_block::Ptr> continue_blocks;
	virtual bool is_same(block::Ptr other) override {
		if (!isa<while_stmt>(other))
			return false;
		while_stmt::Ptr other_stmt = to<while_stmt>(other);
		if (!body->is_same(other_stmt->body))
			return false;
		if (!cond->is_same(other_stmt->cond))
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		np->body = clone(body);
		np->cond = clone(cond);
		return np;
	}
};
class for_stmt : public stmt {
public:
	typedef std::shared_ptr<for_stmt> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<for_stmt>());
	}
	stmt::Ptr decl_stmt;
	expr::Ptr cond;
	expr::Ptr update;
	stmt::Ptr body;

	virtual bool is_same(block::Ptr other) override {
		if (!isa<for_stmt>(other))
			return false;
		for_stmt::Ptr other_stmt = to<for_stmt>(other);
		if (!decl_stmt->is_same(other_stmt->decl_stmt))
			return false;
		if (!cond->is_same(other_stmt->cond))
			return false;
		if (!update->is_same(other_stmt->update))
			return false;
		if (!body->is_same(other_stmt->body))
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		np->decl_stmt = clone(decl_stmt);
		np->cond = clone(cond);
		np->update = clone(update);
		np->body = clone(body);
		return np;
	}
};
class break_stmt : public stmt {
public:
	typedef std::shared_ptr<break_stmt> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<break_stmt>());
	}
	virtual bool is_same(block::Ptr other) override {
		if (!isa<break_stmt>(other))
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		return np;
	}
};
class continue_stmt : public stmt {
public:
	typedef std::shared_ptr<continue_stmt> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<continue_stmt>());
	}
	virtual bool is_same(block::Ptr other) override {
		if (!isa<continue_stmt>(other))
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		return np;
	}
};

class func_decl : public stmt {
public:
	typedef std::shared_ptr<func_decl> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<func_decl>());
	}
	std::string func_name;
	type::Ptr return_type;
	std::vector<var::Ptr> args;
	stmt::Ptr body;
	virtual bool is_same(block::Ptr other) override {
		// Functions don't have static offsets
		if (!isa<func_decl>(other))
			return false;
		func_decl::Ptr other_func = to<func_decl>(other);
		if (!return_type->is_same(other_func->return_type))
			return false;
		if (args.size() != other_func->args.size())
			return false;
		for (unsigned int i = 0; i < args.size(); i++) {
			if (!args[i]->var_type->is_same(other_func->args[i]->var_type))
				return false;
		}
		if (!body->is_same(other_func->body))
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		np->func_name = func_name;
		np->return_type = clone(return_type);
		for (auto arg: args) {
			np->args.push_back(clone(arg));
		}
		np->body = clone(body);
		return np;
	}
};

class return_stmt : public stmt {
public:
	typedef std::shared_ptr<return_stmt> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<return_stmt>());
	}
	expr::Ptr return_val;

	virtual bool is_same(block::Ptr other) override {
		return_stmt::Ptr other_stmt = to<return_stmt>(other);
		if (!return_val->is_same(other_stmt->return_val))
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		np->return_val = clone(return_val);
		return np;
	}
};
} // namespace block
#endif

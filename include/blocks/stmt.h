#ifndef STMT_H
#define STMT_H
#include "blocks/expr.h"
#include <vector>
#include <set>
namespace block {
class stmt : public block {
public:
	typedef std::shared_ptr<stmt> Ptr;
	std::set<std::string> annotation;
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
	bool is_typedef = false;
	bool is_extern = false;
	bool is_static = false;
	virtual bool is_same(block::Ptr other) override {
		if (!isa<decl_stmt>(other))
			return false;
		decl_stmt::Ptr other_stmt = to<decl_stmt>(other);

		if (is_typedef != other_stmt->is_typedef)
			return false;
		if (is_extern != other_stmt->is_extern)
			return false;
		if (is_static != other_stmt->is_static)
			return false;

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
		np->is_typedef = is_typedef;
		np->is_extern = is_extern;
		np->is_static = is_static;	
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
class case_stmt: public stmt {
public:
	typedef std::shared_ptr<case_stmt> Ptr;
	virtual void dump(std::ostream&, int) override;
	virtual void accept(block_visitor* a) override {
		a->visit(self<case_stmt>());
	}
	// if default, int_const is set to nullptr;
	bool is_default;
	int_const::Ptr case_value;
	// Can be null in case the branch is empty
	stmt::Ptr branch;

	virtual bool is_same(block::Ptr other) override {
		if (!isa<case_stmt>(other))
			return false;
		case_stmt::Ptr other_stmt = to<case_stmt>(other);
		if (is_default != other_stmt->is_default)
			return false;
		if (!is_default && !case_value->is_same(other_stmt->case_value))
			return false;
		if (branch != nullptr && other_stmt->branch == nullptr)
			return false;
		if (branch == nullptr && other_stmt->branch != nullptr)
			return false;
		if (branch!= nullptr && !branch->is_same(other_stmt->branch))
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto nc = clone_stmt(this);
		nc->is_default = is_default;
		if (case_value) 
			nc->case_value = clone(case_value);
		else 
			nc->case_value = nullptr;
		if (branch)
			nc->branch = clone(branch);
		else
			nc->branch = nullptr;
		return nc;
	}
};

class switch_stmt: public stmt {
public:
	typedef std::shared_ptr<switch_stmt> Ptr;
	virtual void dump(std::ostream&, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<switch_stmt>());
	}
	expr::Ptr cond;	
	std::vector<case_stmt::Ptr> cases;
	
	virtual bool is_same(block::Ptr other) override {
		if (!isa<switch_stmt>(other))
			return false;
		switch_stmt::Ptr other_stmt = to<switch_stmt>(other);
		if (!cond->is_same(other_stmt->cond))
			return false;
		if (cases.size() != other_stmt->cases.size())
			return false;
		for (unsigned i = 0; i < cases.size(); i++) {
			if (!(cases[i]->is_same(other_stmt->cases[i]))) 
				return false;
		}
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto nc = clone_stmt(this);
		nc->cond = clone(cond);	
		for (auto c: cases) {
			nc->cases.push_back(clone(c));
		}	
		return nc;
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

	bool is_decl_only = false;
	bool is_variadic = false;
	bool is_static = false;
	bool is_inline = false;

	virtual bool is_same(block::Ptr other) override {
		// Functions don't have static offsets
		if (!isa<func_decl>(other))
			return false;
		func_decl::Ptr other_func = to<func_decl>(other);

		if (is_decl_only != other_func->is_decl_only) 
			return false;
		if (is_variadic != other_func->is_variadic) 
			return false;
		if (is_static != other_func->is_static) 
			return false;
		if (is_inline != other_func->is_inline) 
			return false;

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

		np->is_decl_only = is_decl_only;
		np->is_variadic = is_variadic;
		np->is_static = is_static;
		np->is_inline = is_inline;

		for (auto arg: args) {
			np->args.push_back(clone(arg));
		}
		np->body = clone(body);
		return np;
	}
};


class struct_decl : public stmt {
public:
	typedef std::shared_ptr<struct_decl> Ptr;
	virtual void dump(std::ostream&, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<struct_decl>());
	}

	std::string struct_name;
	std::vector<decl_stmt::Ptr> members;
	bool is_union = false;
	bool is_decl_only = false;
	virtual bool is_same(block::Ptr other) override {
		// Struct decls like Function Decls don't have static offsets
		if (!isa<struct_decl>(other)) 
			return false;
		struct_decl::Ptr other_struct = to<struct_decl>(other);

		if (struct_name != other_struct->struct_name)
			return false;

		if (is_union != other_struct->is_union) 
			return false;

		if (is_decl_only != other_struct->is_decl_only) 
			return false;

		if (members.size() != other_struct->members.size()) 
			return false;
		// We will look for exact struct matches including member names
		// This could be relaxed to compare only types with a flag
		for (unsigned int i = 0; i < members.size(); i++) {
			if (!members[i]->is_same(other_struct->members[i]))
				return false;
		}
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_stmt(this);
		np->struct_name = struct_name;
		np->is_union = is_union;
		np->is_decl_only = is_decl_only;
		for (auto mem: members) {
			np->members.push_back(clone(mem));
		}
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

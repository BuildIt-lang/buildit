#ifndef VAR_H
#define VAR_H

#include "blocks/block.h"
#include "util/printer.h"

namespace block {
class type : public block {
public:
	typedef std::shared_ptr<type> Ptr;

	bool is_const = false;
	bool is_volatile = false;

	virtual void accept(block_visitor *a) override {
		a->visit(self<type>());
	}
	virtual void dump(std::ostream &, int) override;
};

template <typename T>
std::shared_ptr<T> clone_type(T* t) {
	auto np = clone_obj(t);
	np->is_const = t->is_const;
	np->is_volatile = t->is_volatile;
	return np;	
}

class scalar_type : public type {
public:
	typedef std::shared_ptr<scalar_type> Ptr;
	enum {
		SHORT_INT_TYPE,
		UNSIGNED_SHORT_INT_TYPE,
		INT_TYPE,
		UNSIGNED_INT_TYPE,
		LONG_INT_TYPE,
		UNSIGNED_LONG_INT_TYPE,
		LONG_LONG_INT_TYPE,
		UNSIGNED_LONG_LONG_INT_TYPE,
		CHAR_TYPE,
		UNSIGNED_CHAR_TYPE,
		VOID_TYPE,
		FLOAT_TYPE,
		DOUBLE_TYPE,
		BOOL_TYPE
	} scalar_type_id;
	virtual void accept(block_visitor *a) override {
		a->visit(self<scalar_type>());
	}
	virtual void dump(std::ostream &, int) override;

	virtual block::Ptr clone_impl(void) override {
		auto np = clone_type(this);
		np->scalar_type_id = scalar_type_id;
		return np;
	}
};

class pointer_type : public type {
public:
	typedef std::shared_ptr<pointer_type> Ptr;
	type::Ptr pointee_type;
	virtual void accept(block_visitor *a) override {
		a->visit(self<pointer_type>());
	}
	virtual void dump(std::ostream &, int) override;
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_type(this);
		np->pointee_type = clone(pointee_type);
		return np;
	}
};

class reference_type : public type {
public:
	typedef std::shared_ptr<reference_type> Ptr;
	type::Ptr referenced_type;
	virtual void accept(block_visitor *a) override {
		a->visit(self<reference_type>());
	}
	virtual void dump(std::ostream &, int) override;
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_type(this);
		np->referenced_type = clone(referenced_type);
		return np;
	}
};

class function_type : public type {
public:
	typedef std::shared_ptr<function_type> Ptr;
	virtual void accept(block_visitor *a) override {
		a->visit(self<function_type>());
	}

	type::Ptr return_type;
	std::vector<type::Ptr> arg_types;

	virtual void dump(std::ostream &, int) override;
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_type(this);
		np->return_type = clone(return_type);
		for (auto t: arg_types) {
			np->arg_types.push_back(clone(t));
		}
		return np;
	}
};
class array_type : public type {
public:
	typedef std::shared_ptr<array_type> Ptr;
	virtual void accept(block_visitor *a) override {
		a->visit(self<array_type>());
	}
	type::Ptr element_type;
	int size;

	virtual void dump(std::ostream &, int) override;
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_type(this);
		np->element_type = clone(element_type);
		np->size = size;
		return np;
	}
};

// Types for complete closure
class builder_var_type : public type {
public:
	typedef std::shared_ptr<builder_var_type> Ptr;
	enum { DYN_VAR, STATIC_VAR } builder_var_type_id;
	virtual void accept(block_visitor *a) override {
		a->visit(self<builder_var_type>());
	}
	type::Ptr closure_type;

	virtual void dump(std::ostream &, int) override;
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_type(this);
		np->builder_var_type_id = builder_var_type_id;
		np->closure_type = clone(closure_type);
		return np;
	}
};

class named_type : public type {
public:
	typedef std::shared_ptr<named_type> Ptr;
	std::string type_name;
	std::vector<type::Ptr> template_args;
	virtual void accept(block_visitor *a) override {
		a->visit(self<named_type>());
	}

	virtual void dump(std::ostream &, int) override;

	virtual block::Ptr clone_impl(void) override {
		auto np = clone_type(this);
		np->type_name = type_name;
		for (auto t: template_args) {
			np->template_args.push_back(clone(t));
		}
		return np;
	}
};

class var : public block {
public:
	typedef std::shared_ptr<var> Ptr;
	virtual void dump(std::ostream &, int) override;
	virtual void accept(block_visitor *a) override {
		a->visit(self<var>());
	}
	// Optional var_name
	std::string var_name;
	std::string preferred_name;
	type::Ptr var_type;
	virtual bool is_same(block::Ptr other) override {
		if (!isa<var>(other))
			return false;

		// It is important to check static tags for variables instead of
		// names because this happens during extraction
		// Also names are dependent on static tags, so it should be fine

		// We also don't need to check if the type is the same, if the tags
		// are the same, the types should be the same.
		// Checking static_offset here should not mess with pattern matchers
		if (static_offset != other->static_offset)
			return false;
		return true;
	}
	virtual block::Ptr clone_impl(void) override {
		auto np = clone_obj(this);
		np->var_name = var_name;
		np->preferred_name = preferred_name;
		np->var_type = clone(var_type);
		return np;
	}
};

} // namespace block
#endif

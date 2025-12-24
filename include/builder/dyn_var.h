#ifndef BUILDER_DYN_VAR_H
#define BUILDER_DYN_VAR_H

#include "builder/block_type_extractor.h"
#include "builder/run_states.h"
#include "builder/to_expr.h"
#include "builder/providers_dyn_var.h"
#include "builder/operator_overload.h"

namespace block {
// Forward declaring this class for friend
class c_code_generator;
}

namespace builder {


// Top level class forward declaration
template <typename T>
class dyn_var;

// Non-typed base class for all dyn_vars
class dyn_var_base {
protected:
	// All members are part of the base class
	// so operations can be done without knowing the 
	// derived type
	enum dyn_var_mode {
		standalone_var,
		member_var,
		compound_expr,
	} var_mode;

	block::var::Ptr block_var;
	block::decl_stmt::Ptr block_decl_stmt;
	dyn_var_base* parent_var;
	std::string member_name;
	block::expr::Ptr block_expr;
	// Required for custom types
	std::vector<block::var::Ptr> user_defined_members;
	int member_counter = 0;

public:
	// Member functions user can call
	void set_type(type t);

public:
	~dyn_var_base() = default;
	friend block::expr::Ptr to_expr(const dyn_var_base& d);
	
	// These friend declarations
	// are added to different T can access each others
	// members
	template <typename T>
	friend class dyn_var_impl;
	
	template <typename T>
	friend class dyn_var;

	template <typename T>
	friend void resize_arr(const dyn_var<T[]> &x, int size);

	friend type type_of(const dyn_var_base& v);

	template <typename T, typename TO>
	friend typename std::enable_if<std::is_same<T, generic>::value>::type 
		copy_types_provider(dyn_var_impl<T>&, const TO&);

	friend class block::c_code_generator;
};

// The dyn_var_impl class has all the implementation
// But still uses dyn_var internally, this way dyn_var doesn't
// have to implement stuff differently
template <typename T>
class dyn_var_impl: public dyn_var_base {

private:	
	// Helper functions that depend on the type
	tracer::tag create_block_var(std::string vname = "", dyn_var_base* supplied_parent = nullptr) {
		block_var = std::make_shared<block::var>();
		block_var->var_type = type_extractor<T>::extract_type();
		if (parents_stack && parents_stack->size() != 0 && supplied_parent == nullptr) {
			supplied_parent = parents_stack->back();
		}

		// First check if this variable is being created as a member	
		if (supplied_parent) {
			var_mode = member_var;	
			parent_var = supplied_parent;
			if (vname == "") {
				member_name = "mem" + std::to_string(parent_var->member_counter++);
			} else {
				member_name = vname;
			}
			if (user_defined_provider_track_members) {
				// Record the block var, record it
				// and then discard
				block_var->var_name = member_name;
				parent_var->user_defined_members.push_back(block_var);
			}
			// Discard the created block_var
			block_var = nullptr;
		} else {
			block_var->var_name = vname;	
			var_mode = standalone_var;
		}

		if (is_under_run() && block_var) {
			// If a variable is created outside of a run, 
			// it doesn't need a static tag
			tracer::tag offset = tracer::get_offset_in_function();
			get_run_state()->insert_live_dyn_var(offset);
			block_var->static_offset = offset;
			return offset;	
		} else {
			// Else return an empty tag
			return tracer::tag();
		}
	}

	void create_var_decl_stmt(block::expr::Ptr init_expr, tracer::tag t) {
		// Create a decl only if under run
		// Sometimes standalone variables can be created at a global scope
		if (!is_under_run()) {
			return;
		}	
		// If variable isn't a standalone var, don't create a decl
		if (var_mode != standalone_var) 
			return;
		// Remove the init_expr from the sequence before committing
		// so the annotations aren't added to the sub expr
		if (init_expr != nullptr) {
			get_run_state()->remove_node_from_sequence(init_expr);
		}
		get_run_state()->commit_uncommitted();	
		// decl statements are only valid for standalone vars
		auto ds = std::make_shared<block::decl_stmt>();
		// Use the tag supplied, not the one from the block var, they could be different
		ds->static_offset = t;
		ds->decl_var = block_var;
		ds->init_expr = init_expr;
		block_decl_stmt = ds;	
		get_run_state()->add_stmt_to_current_block(ds, true);
	}

	void create_standalone(block::expr::Ptr init_expr = nullptr) {
		auto t = create_block_var();	
		create_var_decl_stmt(init_expr, t);
	}
	void move_from (const dyn_var_impl& other) {
		var_mode = other.var_mode;
		block_var = other.block_var;
		block_decl_stmt = other.block_decl_stmt;
		parent_var = other.parent_var;
		block_expr = other.block_expr;
		user_defined_members = other.user_defined_members;			
		member_counter = other.member_counter;
	}
public:
	// An alias to help signature identifer 
	using stored_type = T;
public:
	// General constructors and copy/move constructors	
	dyn_var_impl() {
		create_standalone();	
	}
	dyn_var_impl(const dyn_var_impl& other) {
		create_standalone(to_expr(other));
		copy_types_provider(*this, other);
	}
	dyn_var_impl(dyn_var_impl&& other) { // Move constructor to simply steal resources
		move_from(other);
	}
	// This constructor uses dyn_var<TO> on the RHS
	// because it will never ever be dyn_var_impl
	template <typename TO>
	dyn_var_impl(const dyn_var<TO>& other) {
		create_standalone(to_expr(other));
		copy_types_provider(*this, other);
	}
	template <typename TO>
	dyn_var_impl(dyn_var_impl<TO>&& other) = delete;

	// A general constructor to initialize from any other type
	// TODO: Add check to make sure to_expr exists for the type
	template <typename TO>
	dyn_var_impl(const TO& other) {
		create_standalone(to_expr(other));
	}
	// An std initializer list type can never be deduced, so we
	// need a special constructor for it
	dyn_var_impl(const std::initializer_list<expr_wrapper>& other) {
		create_standalone(to_expr(other));
	}

	// Constructor for creating compound expressions
	template <typename TO, typename V=typename std::enable_if<std::is_base_of<block::expr, TO>::value>::type>
	dyn_var_impl(std::shared_ptr<TO> e) {
		var_mode = compound_expr;
		block_expr = e;		
	}

	// Special constructors with tags

	dyn_var_impl(const defer_init& ) {
		// Defer, everything
	}

	dyn_var_impl(const with_name& wn) {
		auto t = create_block_var(wn.name);
		if (wn.with_decl) {
			create_var_decl_stmt(nullptr, t);
			block_var->preferred_name = "";
		}
	}
	dyn_var_impl(const with_block_var& bv) {
		block_var = bv.var;	
		var_mode = standalone_var;
		if (bv.with_decl) {
			// If a decl is created, it will use the above block var
			create_var_decl_stmt(nullptr, block_var->static_offset);
		}
	}
	// Explicit as_member initialization
	dyn_var_impl(const as_member& am) {
		// No special logic since create_block_var 
		// handles this automatically
		create_block_var(am.member_name, am.parent_var);
	}

	// The with_type constructor is defined is generics
	dyn_var_impl(const with_type&);

	// Operators that need to be defined inside the class
	// Notice that the return type is still dyn_var<T>, not impl
	dyn_var<T>& operator= (const dyn_var_impl<T>& other) {
		auto e1 = to_expr(*this);
		auto e2 = to_expr(other);
		auto e = create_expr<block::assign_expr>({e1, e2});
		e->var1 = e1;
		e->expr1 = e2;
		return *get_invocation_state()->get_arena()->allocate<dyn_var<T>>(e);
	}
	template <typename TO>
	dyn_var<T>& operator= (const TO& other) {
		auto e1 = to_expr(*this);
		auto e2 = to_expr(other);
		auto e = create_expr<block::assign_expr>({e1, e2});
		e->var1 = e1;
		e->expr1 = e2;
		return *get_invocation_state()->get_arena()->allocate<dyn_var<T>>(e); 
	}

	// Use a dummy copy of T to defer the SFINAE check
	// till the operator is actually instantiated
	template <typename TC=T, typename TO>
	dyn_var<op_sq_bkt_ret_provider_t<TC>>& operator[] (const TO& other) {
		using RetType = dyn_var<op_sq_bkt_ret_provider_t<TC>>;
		auto e1 = to_expr(*this);
		auto e2 = to_expr(other);
		auto e = create_expr<block::sq_bkt_expr>({e1, e2});	
		e->var_expr = e1;
		e->index = e2;
		return *get_invocation_state()->get_arena()->allocate<RetType>(e);
	}

	// Unary * operator also uses the same provider
	template <typename TC=T>
	dyn_var<op_sq_bkt_ret_provider_t<TC>>& operator* (void) {
		auto& ret = operator[](0);
		block::expr::Ptr be = ret.block_expr;
		be->setMetadata<bool>("deref_is_star", true);
		return ret;
	}


	// Operator -> also uses the same type, except the return type is a ptr
	// not an lvalue reference	
	template <typename TC=T>
	dyn_var<op_sq_bkt_ret_provider_t<TC>>* operator -> (void) {
		auto &ret = operator*();
		return ret.addr();	
	}	

	template <typename TC=T, typename...Args>
	dyn_var<op_fcall_ret_provider_t<TC>>& operator() (const Args&...args) {
		using RetType = dyn_var<op_fcall_ret_provider_t<TC>>;
		auto e1 = to_expr(*this);
		std::vector<block::expr::Ptr> argv = {to_expr(args)...};
		auto args_copy = argv;
		args_copy.push_back(e1);	
		auto e = create_expr<block::function_call_expr>(args_copy);
		e->expr1 = e1;
		e->args = std::move(argv);
		return *get_invocation_state()->get_arena()->allocate<RetType>(e);
	}

	
	// Bool conversion operator for branching
	explicit operator bool (void) const {
		get_run_state()->commit_uncommitted();
		return get_run_state()->get_next_bool(to_expr(*this));	
	}
public:
	// Destructor for removing variable from live vars
	~dyn_var_impl() {
		if (block_var && is_under_run()) {
			get_run_state()->remove_live_dyn_var(block_var->static_offset);
		}
	}
public:
	// Public functions for users to call	
	// addr function to get the real address
	dyn_var<T>* addr(void) {
		return static_cast<dyn_var<T>*>(this);
	}

	void deferred_init(void) {		
		create_standalone();	
	}
	// A second deferred init to handle any existing constructors
	template <typename TO>
	void deferred_init(const TO& other) {
		move_from(dyn_var_impl<T>(other));		
	}
	void add_attribute(std::string s) {
		std::vector<std::string> attrs;
		if (block_var->hasMetadata<std::vector<std::string>>("attributes")) {
			attrs = block_var->getMetadata<std::vector<std::string>>("attributes");
		}
		if (std::find(attrs.begin(), attrs.end(), s) == attrs.end()) {
			attrs.push_back(s);
		}
		block_var->setMetadata<std::vector<std::string>>("attributes", attrs);
	}
public:
	// static helper functions associated with this dyn_var type
	static block::type::Ptr create_block_type(void) {
		return type_extractor<T>::extract_type();
	}
	
};

// The actual dyn_var type is a simple struct with no new operations
// The user defined member provider is included here and not with the 
// impl so the user can choose to not add them for their custom
// specializations
template <typename T>
class dyn_var: public dyn_var_impl<T>, public user_defined_member_provider<T> {
public:
	using dyn_var_impl<T>::dyn_var_impl;	
	using dyn_var_impl<T>::operator=;
	using dyn_var_impl<T>::operator[];
	using dyn_var_impl<T>::operator();
	using dyn_var_impl<T>::operator bool;
	
	// Some constructors and operators that need to be defined
	dyn_var(): dyn_var_impl<T>() {}
	dyn_var(const dyn_var& other): dyn_var_impl<T>(other) {}	
	dyn_var<T>& operator=(const dyn_var<T>& other) {
		return dyn_var_impl<T>::operator=(other);
	}
};

}

#endif

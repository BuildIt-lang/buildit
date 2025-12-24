#ifndef BUILDER_RUN_STATES_H
#define BUILDER_RUN_STATES_H

#include "util/hash_utils.h"
#include <memory>
#include "util/tracer.h"
#include "builder/forward_declarations.h"
#include "builder/arena_dyn_var.h"
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <functional>
#include "blocks/stmt.h"
#include "builder/tag_factory.h"

namespace builder {

// The classes in this header file will track all dynamic state
// extraction. This moves away from builder_context for tracking all
// state. The builder_context will now be purely used for read only
// parameters

/* There are three levels of data structures maintained based on different 
types of re-runs 

The first is a run_state which is for each execution of the lambda. The run_state
object tracks the generated instructions, the bool vector and the loopback tags. 
This is also the global current run object and will have pointers to the other data 
structures

The second is the execution_state object which tracks the state for an entire program being
invoked. This holds the data structures shared across various runs of the same program that are
created to due to divergent control flow

The final is the an invokation_state which is for maintaining state that is shared across different
runs due to nd_var conflicts. Each call from the user to extract_function_ast creates one of this object

*/

class run_state;
class execution_state;
class invocation_state;
class builder_context;
class nd_var_base;

class run_state {
public:
	static run_state* current_run_state;
private:

	/* Generation related members */

	// The main output block
	block::stmt_block::Ptr current_stmt_block;

	// The uncommitted sequence of expressions
	std::vector<block::block::Ptr> uncommitted_sequence;

	// Cached generated expressions to be reused for children executions
	std::vector<block::expr::Ptr> cached_expr_sequence;
	unsigned int cached_expr_counter = 0;

	// Annotations to be attached to the next statement
	std::set<std::string> current_annotations;

	// Set of dyn_variables that are live
	std::vector<tracer::tag_id> live_dyn_vars;

	/* Tracing and re-execution related members */

	// Vector of bools to return on branching
	std::vector<bool> bool_vector;

	/* Tag creation fields */	
	std::vector<static_var_base*> static_var_tuples;
	std::vector<static_var_base*> deferred_static_var_tuples;
	

	/* Memoization related fields */
	
	// Tags visited before for loopback edges
	std::unordered_map<tracer::tag, block::stmt::Ptr> visited_offsets;	

	// Tag deduplication set, this keeps track of tags 
	// for statements that are the same but the statements are different
	std::unordered_map<tracer::tag, size_t> tag_deduplication_map;
	
	/* Parent dynamic states */
	execution_state* e_state;
	invocation_state* i_state;

public:
	// Run state cannot be created without parent states
	run_state(execution_state* e_state, invocation_state* i_state): e_state(e_state), i_state(i_state) {}	

	execution_state* get_e_state() { return e_state; }	
	invocation_state* get_i_state() { return i_state; }

	bool is_catching_up(void) { return bool_vector.size() > 0; }
	block::expr::Ptr get_next_cached_expr() { 
		assert(cached_expr_counter < cached_expr_sequence.size());
		return cached_expr_sequence[cached_expr_counter++]; 
	}
	void add_to_cached_expr(block::expr::Ptr a) {
		cached_expr_sequence.push_back(a);
	}
	void add_annotation(std::string s) {
		current_annotations.insert(s);
	}
	std::set<std::string> get_and_clear_annotations(void) {
		std::set<std::string> to_ret = current_annotations;
		current_annotations.clear();
		return to_ret;
	}

	void insert_live_dyn_var(const tracer::tag& new_tag);
	void remove_live_dyn_var(const tracer::tag& new_tag);
	
	friend class execution_state;
	friend class invocation_state;
	friend class builder_context;
	friend tracer::tag tracer::get_offset_in_function(void);
	template <typename T>
	friend class static_var;

private:

public:
	void add_stmt_to_current_block(block::stmt::Ptr, bool check_memoization);
	void commit_uncommitted(void);
	void remove_node_from_sequence(block::expr::Ptr);
	void add_node_to_sequence(block::expr::Ptr);
	bool is_visited_tag(tracer::tag &new_tag);
	void erase_tag(tracer::tag &erase_tag);
	bool get_next_bool(block::expr::Ptr);
};

class execution_state {
	/* Memoization related fields */
	std::unordered_map<tracer::tag, block::stmt_block::Ptr> memoized_tags;
	
	/* Parent dynamic states */
	invocation_state* i_state;

public:
	execution_state(invocation_state* i_state): i_state(i_state) {}
	
	friend class invocation_state;
	friend class run_state;
	friend class builder_context;

};

class invocation_state {
	/* ND_VAR state */
	std::unordered_map<tracer::tag, std::shared_ptr<nd_var_base>> nd_state_map;

	// Tag factory state
	tag_factory tag_factory_instance;

	// Main invocation function
	std::function<void(void)> invocation_function;

	// Generated function declaration
	block::func_decl::Ptr generated_func_decl;

	builder_context* b_ctx = nullptr;
	
	// Arena is part of the invocation state
	// so the buffers persist
	dyn_var_arena var_arena;

public:

	builder_context* get_b_ctx() { return b_ctx; }

	friend class run_state;
	friend class execution_state;
	friend class builder_context;
	
	template <typename F, typename...OtherArgs>
	friend invocation_state extract_signature(F func, OtherArgs&&... args);

	template <typename ProcessedArgTypes, typename RemainingArgTypes, typename ReturnType, typename Enable>
	friend struct extract_signature_impl;

	template <typename T, typename...Args>
	friend std::shared_ptr<T> get_or_create_generator(tracer::tag req_tag, Args&&...args);

public:
	dyn_var_arena* get_arena(void) {
		return &var_arena;
	}
};

static inline run_state* get_run_state(void) {
	assert(run_state::current_run_state != nullptr);
	return run_state::current_run_state;	
}

static inline bool is_under_run(void) {
	return run_state::current_run_state != nullptr;
}

static inline execution_state* get_execution_state(void) {
	return get_run_state()->get_e_state();
}

static inline invocation_state* get_invocation_state(void) {
	return get_run_state()->get_i_state();
}

static inline builder_context* get_builder_context(void) {
	return get_invocation_state()->get_b_ctx();
}

}
#endif

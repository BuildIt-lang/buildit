#include "builder/run_states.h"
#include "builder/exceptions.h"
#include <algorithm>
namespace builder {

run_state* run_state::current_run_state = nullptr;




void run_state::add_stmt_to_current_block(block::stmt::Ptr s, bool check_for_conflicts) {
	if (bool_vector.size() > 0) {
		return;
	}
	if (!current_annotations.empty()) {
		s->annotation = get_and_clear_annotations();
	}
	if (!s->static_offset.is_empty() && is_visited_tag(s->static_offset) > 0) {
		// Let's go and find that statement
		auto lt = visited_offsets[s->static_offset];
		// This is only a loopback if it is an exact match
		if (lt->is_same(s)) 
			throw LoopBackException(s->static_offset);

		// We have found a tag is the same, but statemetns aren't the same
		// The tag we have has dedup_id as 0	
		tracer::tag tag0 = s->static_offset;

		if (tag_deduplication_map.find(tag0) == tag_deduplication_map.end()) {
			// If duplicates aren't seen before, insert this tag in the deduplication_map
			// 1 is HOW many such tags exist, default is 1 for all tags
			tag_deduplication_map[tag0] = 1;
		}
		// We have already checked 0
		size_t d_id = 1, max_d_id = tag_deduplication_map[tag0];
		for (d_id = 1; d_id < max_d_id; d_id++) {
			tag0.dedup_id = d_id;
			// Find the statement
			auto lt = visited_offsets[tag0];
			if (lt->is_same(s)) {
				s->static_offset = tag0;
				s->static_offset.cached_string = "";
				throw LoopBackException(s->static_offset);
			}
		}
		// If we reached here, there is no match, this must be a new copy, update the tag and dedup map
		s->static_offset.dedup_id = d_id;
		s->static_offset.cached_string = "";
		tag0.dedup_id = 0;
		tag_deduplication_map[tag0] = d_id + 1;	
	}

	tracer::tag stag = s->static_offset;

	if (e_state->memoized_tags.find(stag) != e_state->memoized_tags.end() && check_for_conflicts && 
		bool_vector.size() == 0) {
		// This tag has been seen on some other execution. We can reuse.
		// First find the tag -
		block::stmt_block::Ptr parent = e_state->memoized_tags[stag];
		unsigned int i = 0;
		for (i = 0; i < parent->stmts.size(); i++) {
			if (parent->stmts[i]->static_offset == s->static_offset)
				break;
		}
		// Special case of stmt expr and if_stmt
		if (block::isa<block::expr_stmt>(s) && block::isa<block::if_stmt>(parent->stmts[i])) {
			block::if_stmt::Ptr p_stmt = block::to<block::if_stmt>(parent->stmts[i]);
			block::expr_stmt::Ptr expr = block::to<block::expr_stmt>(s);

			if (p_stmt->cond->is_same(expr->expr1))
				throw MemoizationException(s->static_offset, parent, i);
		}


		if (parent->stmts[i]->is_same(s))
			throw MemoizationException(s->static_offset, parent, i);
	}
	// If dedup happens, this has already been updated
	visited_offsets[s->static_offset] = s;
	current_stmt_block->stmts.push_back(s);
}

bool run_state::is_visited_tag(tracer::tag &new_tag) {
	if (visited_offsets.find(new_tag) != visited_offsets.end())
		return true;
	return false;
}

void run_state::erase_tag(tracer::tag &erase_tag) {
	visited_offsets.erase(erase_tag);
}
void run_state::commit_uncommitted(void) {
	for (auto block_ptr : uncommitted_sequence) {
		// if it has been removed by setting it to nullptr, skip
		if (block_ptr == nullptr) continue;
		block::expr_stmt::Ptr s = std::make_shared<block::expr_stmt>();
		assert(block::isa<block::expr>(block_ptr));
		s->static_offset = block_ptr->static_offset;
		s->expr1 = block::to<block::expr>(block_ptr);
		assert(current_stmt_block != nullptr);
		add_stmt_to_current_block(s, true);
	}
	uncommitted_sequence.clear();
}
void run_state::remove_node_from_sequence(block::expr::Ptr e) {
	// At this point, there is a chance a statement _might_ have been committed if
	// a variable was declared. This happens when you return a dyn_var from a function
	// Now this is not particularly bad because it just leaves a stray expression in the
	// generated code, but 1. it can mess with some pattern matchers, 2. could have
	// unexpected side effects, so we are going to do a clean up just to be sure
	// So we will check if the expr that we are trying to delete is in the uncommitted
	// sequence, if not we will try to find for it in the committed expressions

	bool found = false;
	for (unsigned i = 0; i < uncommitted_sequence.size(); i++) {
		if (uncommitted_sequence[i] == e) {
			uncommitted_sequence[i] = nullptr;
			found = true;
		}
	}
	if (!found) {
		// Could be committed already
		// It is safe to update the parent block here, because the memoization doesn't care about indices
		// But don't actually delete the statement, because there could be gotos that are jumping here
		// instead just mark it for deletion later
		for (auto stmt : current_stmt_block->stmts) {
			if (block::isa<block::expr_stmt>(stmt)) {
				auto expr_s = block::to<block::expr_stmt>(stmt);
				if (expr_s->expr1 == e) {
					expr_s->mark_for_deletion = true;
				}
			}
		}
	}
}
void run_state::add_node_to_sequence(block::expr::Ptr e) {
	uncommitted_sequence.push_back(e);
}

bool run_state::get_next_bool(block::expr::Ptr expr) {
	commit_uncommitted();
	if (bool_vector.size() == 0) {
		tracer::tag offset = expr->static_offset;
		throw OutOfBoolsException(offset);
	}
	bool ret_val = bool_vector.back();
	bool_vector.pop_back();
	return ret_val;
}

void run_state::insert_live_dyn_var(const tracer::tag& t) {
	// First convert the tag to tag_id using the invocation's tag factory
	tracer::tag_id tid = i_state->tag_factory_instance.create_tag_id(t);
	// Insert it into the live set and sort
	live_dyn_vars.push_back(tid);
	std::sort(live_dyn_vars.begin(), live_dyn_vars.end());
}
void run_state::remove_live_dyn_var(const tracer::tag& t) {
	// First convert the tag to tag_id using the invocation's tag factory
	tracer::tag_id tid = i_state->tag_factory_instance.create_tag_id(t);

	// Search using binary search, might not be exact
	auto it = std::lower_bound(live_dyn_vars.begin(), live_dyn_vars.end(), tid);

	// Check if we actually found the value
	if (it != live_dyn_vars.end() && *it == tid) {
		// Erase it. This shifts all subsequent elements left.
		live_dyn_vars.erase(it); 
	}	
}

}

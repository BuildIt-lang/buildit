#include "util/tracer.h"
#include "builder/builder_context.h"
#include "builder/static_var.h"
#include <string>

#ifdef TRACER_USE_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif

namespace builder {
extern void lambda_wrapper(std::function<void(void)>);
extern void lambda_wrapper_close(void);
} // namespace builder
namespace tracer {
#ifdef TRACER_USE_LIBUNWIND
tag get_offset_in_function_impl(builder::builder_context *current_builder_context) {
	unsigned long long function = (unsigned long long)(void *)builder::lambda_wrapper;
	unsigned long long function_end = (unsigned long long)(void *)builder::lambda_wrapper_close;
	unw_context_t context;
	unw_cursor_t cursor;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);

	tag new_tag;
	while (unw_step(&cursor)) {
		unw_word_t ip;
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		if ((unsigned long long)ip >= function && (unsigned long long)ip < function_end)
			break;
		new_tag.pointers.push_back((unsigned long long)ip);
	}
	// Now add snapshots of static vars
	assert(current_builder_context != nullptr);

	for (builder::tracking_tuple tuple : current_builder_context->deferred_static_var_tuples) {
		new_tag.static_var_snapshots.push_back(tuple.snapshot());
		if (builder::builder_context::current_builder_context->enable_d2x) {
			new_tag.static_var_key_values.push_back({tuple.var_ref->var_name, tuple.var_ref->serialize()});
		}
	}
	for (builder::tracking_tuple tuple : current_builder_context->static_var_tuples) {
		new_tag.static_var_snapshots.push_back(tuple.snapshot());
		if (builder::builder_context::current_builder_context->enable_d2x) {
			new_tag.static_var_key_values.push_back({tuple.var_ref->var_name, tuple.var_ref->serialize()});
		}
	}
	return new_tag;
}

#else
tag get_offset_in_function_impl(builder::builder_context *current_builder_context) {
	unsigned long long function = (unsigned long long)(void *)builder::lambda_wrapper;
	unsigned long long function_end = (unsigned long long)(void *)builder::lambda_wrapper_close;

	void *buffer[50];
	tag new_tag;
	// First add the RIP pointers
	int backtrace_size = backtrace(buffer, 50);
	for (int i = 0; i < backtrace_size; i++) {
		if ((unsigned long long)buffer[i] >= function && (unsigned long long)buffer[i] < function_end)
			break;
		new_tag.pointers.push_back((unsigned long long)buffer[i]);
	}

	// Now add snapshots of static vars
	assert(current_builder_context != nullptr);

	for (builder::tracking_tuple tuple : current_builder_context->deferred_static_var_tuples) {
		new_tag.static_var_snapshots.push_back(tuple.snapshot());
		if (builder::builder_context::current_builder_context->enable_d2x) {
			new_tag.static_var_key_values.push_back({tuple.var_ref->var_name, tuple.var_ref->serialize()});
		}
	}
	for (builder::tracking_tuple tuple : current_builder_context->static_var_tuples) {
		new_tag.static_var_snapshots.push_back(tuple.snapshot());
		if (builder::builder_context::current_builder_context->enable_d2x) {
			new_tag.static_var_key_values.push_back({tuple.var_ref->var_name, tuple.var_ref->serialize()});
		}
	}
	return new_tag;
}
#endif
static unsigned long long unique_tag_counter = 0;
tag get_unique_tag(void) {
	tag new_tag;
	new_tag.pointers.push_back(0);
	new_tag.pointers.push_back(unique_tag_counter);
	unique_tag_counter++;
	return new_tag;
}

} // namespace tracer

#include "builder/builder.h"

namespace builder {
void annotate(std::string label) {
	builder_context::current_builder_context->commit_uncommitted();
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return;
	builder_context::current_builder_context->current_label = label;
}
} // namespace builder

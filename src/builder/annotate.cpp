#include "builder/builder.h"

namespace builder {
	void annotate(std::string label) {
		builder_context::current_builder_context->commit_uncommitted();
		builder_context::current_builder_context->current_label = label;				
	}
}

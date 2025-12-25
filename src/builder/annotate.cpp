#include "builder/run_states.h"

namespace builder {
void annotate(std::string label) {
	get_run_state()->commit_uncommitted();
	if (get_run_state()->is_catching_up())
		return;
	get_run_state()->add_annotation(label);
}
} // namespace builder

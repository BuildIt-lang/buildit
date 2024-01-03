#ifndef TRACER_H
#define TRACER_H
#include <execinfo.h>
#include <string>
#include <vector>

#define MAX_TRACKING_VAR_SIZE (128)
namespace builder {
class builder_context;
}

namespace tracer {

class tag {
public:
	std::vector<unsigned long long> pointers;
	std::vector<std::string> static_var_snapshots;
	std::vector<std::pair<std::string, std::string>> static_var_key_values;

	bool operator==(const tag &other) {
		if (other.pointers.size() != pointers.size())
			return false;
		for (unsigned int i = 0; i < pointers.size(); i++)
			if (pointers[i] != other.pointers[i])
				return false;
		if (other.static_var_snapshots.size() != static_var_snapshots.size())
			return false;
		for (unsigned int i = 0; i < static_var_snapshots.size(); i++)
			if (static_var_snapshots[i] != other.static_var_snapshots[i])
				return false;
		return true;
	}
	bool operator!=(const tag &other) {
		return !operator==(other);
	}
	bool is_empty(void) {
		return pointers.size() == 0;
	}
	void clear(void) {
		pointers.clear();
		static_var_snapshots.clear();
	}
	std::string stringify(void) {
		std::string output_string = "[";
		for (unsigned int i = 0; i < pointers.size(); i++) {
			char temp[128];
			sprintf(temp, "%llx", pointers[i]);
			output_string += temp;
			if (i != pointers.size() - 1)
				output_string += ", ";
		}
		output_string += "]:[";
		for (unsigned int i = 0; i < static_var_snapshots.size(); i++) {
			output_string += static_var_snapshots[i];
			if (i != static_var_snapshots.size() - 1)
				output_string += ", ";
		}
		output_string += "]";

		return output_string;
	}

	std::string stringify_loc(void) {
		std::string output_string = "[";
		for (unsigned int i = 0; i < pointers.size(); i++) {
			char temp[128];
			sprintf(temp, "%llx", pointers[i]);
			output_string += temp;
			if (i != pointers.size() - 1)
				output_string += ", ";
		}
		output_string += "]:[";
		output_string += "]";

		return output_string;
	}
	std::string stringify_stat(void) {
		std::string output_string = "[";
		output_string += "]:[";
		for (unsigned int i = 0; i < static_var_snapshots.size(); i++) {
			output_string += static_var_snapshots[i];
			if (i != static_var_snapshots.size() - 1)
				output_string += ", ";
		}
		output_string += "]";

		return output_string;
	}
};

tag get_unique_tag(void);

tag get_offset_in_function_impl(builder::builder_context *current_builder_context);

} // namespace tracer
#endif

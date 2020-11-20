#ifndef BUILDER_UNION_H
#define BUILDER_UNION_H

namespace builder {

template <typename T>
class builder_union {
public:
	enum class type_t {
		STATIC_VAR,
		DYN_VAR,
		BUILDER,

		UNDECIDED,
	};

	type_t current_type;

	static_var<T> *wrapped_static_var = nullptr;
	dyn_var<T> *wrapped_dyn_var = nullptr;

	builder *wrapped_builder = nullptr;

	~builder_union() {
		if (current_type == type_t::STATIC_VAR) {
			delete wrapped_static_var;
		} else if (current_type == type_t::DYN_VAR) {
			delete wrapped_dyn_var;
		}
	}

	builder_union(const T &val) {
		current_type = type_t::STATIC_VAR;
		wrapped_static_var = new static_var<T>();
		*wrapped_static_var = val;
	}
	builder_union(const builder &val) {
		current_type = type_t::DYN_VAR;
		wrapped_dyn_var = new dyn_var<T>(val);
	}
	builder_union(const builder_union &val) {
		current_type = val.current_type;
		if (current_type == type_t::STATIC_VAR) {
			wrapped_static_var = new static_var<T>();
			*wrapped_static_var = *(val.wrapped_static_var);
		}
	}
	builder_union() {
		// Determine type on first assignment
		current_type = type_t::UNDECIDED;
	}
};

} // namespace builder

#endif

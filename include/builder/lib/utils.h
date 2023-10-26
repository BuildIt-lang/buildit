#ifndef BUILDER_LIB_UTILS
#define BUILDER_LIB_UTILS
#include "blocks/block.h"

namespace builder {
template <typename T>
static_var<T> up_cast_range(dyn_var<T> &v, T range) {
	static_var<T> s;
	for (s = 0; s < range - 1; s++) {
		if (v == s) {
			return s;
		}
	}
	return s;
}

template <typename T>
void resize_arr(const dyn_var<T[]> &x, int size) {
	block::to<block::array_type>(x.block_var->var_type)->size = size;
}

} // namespace builder
#endif

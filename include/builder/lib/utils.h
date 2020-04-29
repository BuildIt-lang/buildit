#ifndef BUILDER_LIB_UTILS
#define BUILDER_LIB_UTILS

namespace builder {
static_var<int> up_cast_range(dyn_var<int> &v, int range) {
        static_var<int> s;
        for (s = 0; s < range-1; s++) {
                if (s == v) {
                        return s;
                }
        }
	return s;
}

}
#endif

#include "blocks/generic_checker.h"
#include <assert.h>
namespace block {

void generic_null_checker::visit(var::Ptr v) {
	if (v->var_type == nullptr) {
		assert(false && "Variable has a NULL type. Check all generics that are initialized without types");
	}
}

}

#ifndef BUILDER_H
#define BUILDER_H
#include "builder/builder_base.h"
#include "builder/operator_overload.h"

namespace builder {

class builder: public builder_base<builder> {
public:
	using builder_base<builder>::builder_base;

	builder operator=(const builder &a) {
		return assign(a);
	}
	using builder_base<builder>::operator[];
};


template <typename MT>
class builder_final: public builder_base<builder_final<MT>>, public MT {	
public:
	using builder_base<builder_final<MT>>::builder_base;
	using builder_base<builder_final<MT>>::operator[];
	builder_final<MT> operator=(const builder_final<MT> &a) {
		return this->assign(a);
	}
	
	virtual block::expr::Ptr get_parent() const {
		return this->block_expr;
	}
	
	builder_final<MT>(const builder_final<MT> &a): builder_base<builder_final<MT>>(a), MT() {
		this->block_expr = a.block_expr;
	}
	
};


}


#endif

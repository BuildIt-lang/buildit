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


}


#endif

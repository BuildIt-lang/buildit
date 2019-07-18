#ifndef BUILDER_H
#define BUILDER_H
#include <memory>
#include <strings>
#include "blocks/var.h"
#include "builder/builder_context.h"

namespace builder {
// Builder objects are always alive only for duration of the RUN/SEQUENCE. 
// Never store pointers to these objects (across runs) or heap allocate them.

class builder {
public:
	builder_context* context;
	builder(builder_context* context_): context(context_) {}	
	builder() = delete;
};


class var: builder {
public:
	
	// Optional var name
	std::string var_name;
	block::var block_var;
	var() = delete;
	

};


class int_var: var {
public:
	int_var() = delete;
	int_var(builder_context* context_) {
		builder::builder(context_);
	}	
};


}
#endif

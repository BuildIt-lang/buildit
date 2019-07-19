#ifndef BUILDER_H
#define BUILDER_H
#include <memory>
#include <string>
#include "blocks/var.h"
#include "builder/builder_context.h"

namespace builder {
// Builder objects are always alive only for duration of the RUN/SEQUENCE. 
// Never store pointers to these objects (across runs) or heap allocate them.

class builder {
public:
	builder_context* context;
	builder() = default;
	builder(builder_context* context_): context(context_) {}	
};


class var: public builder {
public:
	
	// Optional var name
	std::string var_name;
	block::var::Ptr block_var;
	
	operator block::expr::Ptr () const;


};
block::expr::Ptr operator && (const var&, const block::expr::Ptr&);


class int_var: public var {
public:
	int_var(builder_context* context_);
};


}
#endif

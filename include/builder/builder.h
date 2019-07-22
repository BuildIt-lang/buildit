#ifndef BUILDER_H
#define BUILDER_H
#include <memory>
#include <string>
#include "blocks/var.h"
#include "builder/builder_context.h"

namespace builder {
// Builder objects are always alive only for duration of the RUN/SEQUENCE. 
// Never store pointers to these objects (across runs) or heap allocate them.
class var;
class builder {
public:
	
	block::expr::Ptr block_expr;
	template <typename T>	
	builder builder_binary_op(const builder &);
	template <typename T>	
	builder builder_unary_op();
	builder operator && (const builder &);	
	builder operator || (const builder &);
	builder operator + (const builder &);
	builder operator - (const builder &);
	builder operator * (const builder &);
	builder operator / (const builder &);
	
	builder operator ! ();
	operator bool();

};


class var {
public:
	// Optional var name
	std::string var_name;
	block::var::Ptr block_var;

	
	operator builder() const;

	operator bool();
	builder operator && (const builder &);
	builder operator || (const builder &);
	builder operator + (const builder &);
	builder operator - (const builder &);
	builder operator * (const builder &);
	builder operator / (const builder &);
	
	builder operator = (const builder &);
	builder operator ! ();
};


class int_var: public var {
public:
	using var::operator =;
	int_var();
};
}

#endif

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
	builder() = default;	
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
	explicit operator bool();

	builder (const int&);

};
builder operator && (const int &a, const builder &);
builder operator || (const int &a, const builder &);
builder operator + (const int &a, const builder &);
builder operator - (const int &a, const builder &);
builder operator * (const int &a, const builder &);
builder operator / (const int &a, const builder &);

class var {
public:
	// Optional var name
	std::string var_name;
	block::var::Ptr block_var;
	block::decl_stmt::Ptr block_decl_stmt;

	var() = default;
	
	operator builder() const;

	explicit operator bool();
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
	void create_int_var(void);
	int_var();
	int_var(const int_var&);
	int_var(const builder&);
	int_var(const int);
};

}

#endif

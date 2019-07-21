#ifndef EXPR_H
#define EXPR_H

#include "blocks/block.h"
#include "blocks/var.h"

#include <string>
#include <memory>
namespace block{
class expr: public block {
public:
	typedef util::wrapped_shared_ptr<expr> Ptr; 		
	virtual void dump(std::ostream&, int);
};
}
namespace builder{
bool get_next_bool_from_builder_context(builder_context *context, block::expr::Ptr expr);
}
namespace util {
template <>
inline wrapped_shared_ptr<block::expr>::operator bool() {
	return get_next_bool_from_builder_context(get()->context, *this);
}

}

namespace block {
class unary_expr: public expr {
public:
	typedef util::wrapped_shared_ptr<unary_expr> Ptr;
	expr::Ptr expr1;
	virtual void dump(std::ostream&, int);
	
};

class binary_expr: public expr {
public:
	typedef util::wrapped_shared_ptr<binary_expr> Ptr;
	
	virtual void dump(std::ostream&, int);
	expr::Ptr expr1;
	expr::Ptr expr2;

};

// For the logical not operator
class not_expr: public unary_expr {
public:
	typedef util::wrapped_shared_ptr<not_expr> Ptr;
	virtual void dump(std::ostream&, int);
};


class and_expr: public binary_expr {
public: 
	typedef util::wrapped_shared_ptr<and_expr> Ptr;	
	virtual void dump(std::ostream&, int);
};

expr::Ptr operator && (const expr::Ptr&, const expr::Ptr&);

class or_expr: public binary_expr {
public:
	typedef util::wrapped_shared_ptr<or_expr> Ptr;
	virtual void dump(std::ostream&, int);
};

expr::Ptr operator || (const expr::Ptr&, const expr::Ptr&);

class plus_expr: public binary_expr {
public: 
	typedef util::wrapped_shared_ptr<plus_expr> Ptr;
	virtual void dump(std::ostream&, int);
};

expr::Ptr operator + (const expr::Ptr&, const expr::Ptr&);

class minus_expr: public binary_expr {
public:
	typedef util::wrapped_shared_ptr<minus_expr> Ptr;
	virtual void dump(std::ostream&, int);
};

expr::Ptr operator - (const expr::Ptr&, const expr::Ptr&);

class mul_expr: public binary_expr {
public: 
	typedef util::wrapped_shared_ptr<mul_expr> Ptr;
	virtual void dump(std::ostream&, int);
};

expr::Ptr operator * (const expr::Ptr&, const expr::Ptr&);

class div_expr: public binary_expr {
public: 
	typedef util::wrapped_shared_ptr<div_expr> Ptr;
	virtual void dump(std::ostream&, int);
};

expr::Ptr operator / (const expr::Ptr&, const expr::Ptr&);

class var_expr: public expr {
public:
	typedef util::wrapped_shared_ptr<var_expr> Ptr;
	virtual void dump(std::ostream&, int);

	var::Ptr var1;	
};

class const_expr: public expr {
public:
	typedef util::wrapped_shared_ptr<const_expr> Ptr;
	virtual void dump(std::ostream&, int);
};

class int_const: public const_expr {
public:
	typedef util::wrapped_shared_ptr<int_const> Ptr;
	virtual void dump(std::ostream&, int);
	
	long long value;
};

class assign_expr: public expr {
public:
	typedef util::wrapped_shared_ptr<assign_expr> Ptr;
	virtual void dump(std::ostream&, int);
	
	var::Ptr var1;	
	expr::Ptr expr1;

};
}

#endif

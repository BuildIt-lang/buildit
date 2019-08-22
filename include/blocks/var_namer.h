#ifndef VAR_NAMER_H
#define VAR_NAMER_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"

namespace block {
class var_namer: public block_visitor {
public:
	int var_counter = 0;
	stmt::Ptr ast;
	virtual void visit(decl_stmt::Ptr);
};
class var_replacer: public block_visitor {
public:
	var::Ptr to_replace;
	tracer::tag offset_to_replace;
	
	//virtual void visit(assign_expr::Ptr);
	virtual void visit(var_expr::Ptr);
	
};
}
#endif

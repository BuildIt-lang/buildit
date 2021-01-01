#ifndef BLOCKS_RCE_H
#define BLOCKS_RCE_H

#include "blocks/block_visitor.h"
#include "blocks/stmt.h"
#include "blocks/expr.h"
namespace block {

class gather_redundant_decls: public block_visitor {
public:
	using block_visitor::visit;
	std::vector<decl_stmt::Ptr> gathered_decls;
	virtual void visit(decl_stmt::Ptr) override;
	virtual void visit(assign_expr::Ptr) override;
};


class replace_redundant_vars: public block_visitor {
public:
	using block_visitor::visit;
	decl_stmt::Ptr to_replace;
	virtual void visit(var_expr::Ptr) override;
	virtual void visit(stmt_block::Ptr) override;
};

void eliminate_redundant_vars(block::Ptr ast);

}

#endif

#ifndef VAR_NAMER_H
#define VAR_NAMER_H

#include "blocks/block_visitor.h"
#include "blocks/block_replacer.h"
#include "blocks/stmt.h"
#include <map>

namespace block {

class var_namer: public block_visitor {
public:
	using block_visitor::visit;
	int var_counter = 0;
	std::map<std::string, var::Ptr> collected_decls;
	std::map<std::string, decl_stmt::Ptr> decls_to_hoist;

	virtual void visit(decl_stmt::Ptr) override;

	static void name_vars(block::Ptr ast);
};

class var_replacer: public block_visitor {
public:
	using block_visitor::visit;
	std::map<std::string, var::Ptr> &collected_decls;
	var_replacer(std::map<std::string, var::Ptr> &d): collected_decls(d) {}
	
	virtual void visit (var_expr::Ptr) override;

};

class var_hoister: public block_replacer {
public:
	using block_replacer::visit;
	std::map<std::string, decl_stmt::Ptr> &decls_to_hoist;
	var_hoister(std::map<std::string, decl_stmt::Ptr> &d): decls_to_hoist(d) {}
	virtual void visit (decl_stmt::Ptr) override;
};

}


#endif

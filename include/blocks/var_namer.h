#ifndef VAR_NAMER_H
#define VAR_NAMER_H

#include "blocks/block_replacer.h"
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"
#include <map>

namespace block {

class var_gather_escapes : public block_visitor {
public:
	using block_visitor::visit;
	std::vector<std::string> &escaping_tags;
	var_gather_escapes(std::vector<std::string> &e) : escaping_tags(e) {}
	virtual void visit(decl_stmt::Ptr) override;
};

class var_namer : public block_visitor {
public:
	using block_visitor::visit;
	int var_counter = 0;
	std::map<std::string, var::Ptr> collected_decls;
	std::map<std::string, decl_stmt::Ptr> decls_to_hoist;
	std::vector<std::string> decl_tags_to_hoist;

	std::vector<std::string> escaping_tags;

	virtual void visit(decl_stmt::Ptr) override;

	static void name_vars(block::Ptr ast);
};

class var_replacer : public block_visitor {
public:
	using block_visitor::visit;
	std::map<std::string, var::Ptr> &collected_decls;
	std::vector<std::string> &escaping_tags;
	var_replacer(std::map<std::string, var::Ptr> &d, std::vector<std::string> &e)
	    : collected_decls(d), escaping_tags(e) {}

	virtual void visit(var_expr::Ptr) override;
};

class var_hoister : public block_replacer {
public:
	using block_replacer::visit;
	std::map<std::string, decl_stmt::Ptr> &decls_to_hoist;
	std::vector<std::string> &escaping_tags;
	var_hoister(std::map<std::string, decl_stmt::Ptr> &d, std::vector<std::string> &e)
	    : decls_to_hoist(d), escaping_tags(e) {}
	virtual void visit(decl_stmt::Ptr) override;
};

class var_reference_promoter : public block_replacer {
public:
	using block_replacer::visit;
	virtual void visit(var_expr::Ptr) override;
};

} // namespace block

#endif

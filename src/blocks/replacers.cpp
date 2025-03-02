#include "blocks/matchers/matchers.h"
#include "blocks/matchers/replacers.h"
#include "blocks/block_replacer.h"
#include "blocks/block.h"
#include "blocks/stmt.h"
#include "blocks/expr.h"
#include "blocks/var.h"
#include <memory>
#include <map>
#include <iostream>
namespace block {
namespace matcher {

static bool replacer_error(bool error, std::string message) {
	if (!error) {
		std::cerr << "PM (replacer) error: " << message << std::endl;
		return false;
	}
	return true;
}
#define REPLACER_ASSERT(expr, message) if (!replacer_error(expr, message)) return nullptr

static block::Ptr create_ast(pattern::Ptr p, const std::map<std::string, block::Ptr>& matches);

static inline expr::Ptr create_expr_ast(pattern::Ptr p, const std::map<std::string, block::Ptr>& matches) {
	block::Ptr b = create_ast(p, matches);
	// Special case, handle var where var_expr are expected
	if (::block::isa<::block::var>(b)) {
		::block::var_expr::Ptr ve = std::make_shared<::block::var_expr>();
		ve->var1 = ::block::to<::block::var>(b);
		b = ve;
	}
	REPLACER_ASSERT(::block::isa<::block::expr>(b), "Replacer does not create an expr");
	return ::block::to<::block::expr>(b);
}

template <typename T>
block::Ptr create_unary_ast(pattern::Ptr p, const std::map<std::string, block::Ptr>& matches) {
	REPLACER_ASSERT(p->children.size() == 1 || p->name != "", "Specialized Unary Expr replacer must have one child or must reference a capture");
	if (p->children.size()) {
		typename T::Ptr t = std::make_shared<T>();
		t->expr1 = create_expr_ast(p->children[0], matches);
		return t;
	}
	return matches.at(p->name);
}


template <typename T>
block::Ptr create_binary_ast(pattern::Ptr p, const std::map<std::string, block::Ptr>& matches) {
	REPLACER_ASSERT(p->children.size() == 2 || p->name != "", "Specialized Binary expr replacer must have 2 children or must reference a capture");
	if (p->children.size() == 2) {
		typename T::Ptr t = std::make_shared<T>();
		t->expr1 = create_expr_ast(p->children[0], matches);
		t->expr2 = create_expr_ast(p->children[1], matches);
		return t;
	}
	return matches.at(p->name);
}


block::Ptr create_ast(pattern::Ptr p, const std::map<std::string, block::Ptr>& matches) {
	// This is a recursive function that will create the whole pattern
	// matches is read-only
	switch (p->type) {
		case pattern::node_type::block: 
			REPLACER_ASSERT(p->name != "", "Generic Block replacer must reference a capture");
			return matches.at(p->name);
		case pattern::node_type::expr:
			REPLACER_ASSERT(p->name != "", "Generic Expr replacer must reference a capture");
			return matches.at(p->name);	
		case pattern::node_type::unary_expr:
			REPLACER_ASSERT(p->name != "", "Generic Unary Expr replacer must reference a capture");
			return matches.at(p->name);
		case pattern::node_type::binary_expr:
			REPLACER_ASSERT(p->name != "", "Generic Binary Expr replacer must reference a capture");
			return matches.at(p->name);
		case pattern::node_type::not_expr:
			return create_unary_ast<::block::not_expr>(p, matches);
		case pattern::node_type::unary_minus_expr:
			return create_unary_ast<::block::unary_minus_expr>(p, matches);
		case pattern::node_type::bitwise_not_expr:
			return create_unary_ast<::block::bitwise_not_expr>(p, matches);
		case pattern::node_type::and_expr:
			return create_binary_ast<::block::and_expr>(p, matches);
		case pattern::node_type::bitwise_and_expr:
			return create_binary_ast<::block::bitwise_and_expr>(p, matches);
		case pattern::node_type::bitwise_or_expr:
			return create_binary_ast<::block::bitwise_or_expr>(p, matches);
		case pattern::node_type::bitwise_xor_expr:
			return create_binary_ast<::block::bitwise_xor_expr>(p, matches);
		case pattern::node_type::lshift_expr:
			return create_binary_ast<::block::lshift_expr>(p, matches);
		case pattern::node_type::rshift_expr:
			return create_binary_ast<::block::rshift_expr>(p, matches);
		case pattern::node_type::or_expr:
			return create_binary_ast<::block::or_expr>(p, matches);
		case pattern::node_type::plus_expr:
			return create_binary_ast<::block::plus_expr>(p, matches);
		case pattern::node_type::minus_expr:
			return create_binary_ast<::block::minus_expr>(p, matches);
		case pattern::node_type::mul_expr:
			return create_binary_ast<::block::mul_expr>(p, matches);
		case pattern::node_type::div_expr:
			return create_binary_ast<::block::div_expr>(p, matches);
		case pattern::node_type::lt_expr:
			return create_binary_ast<::block::lt_expr>(p, matches);
		case pattern::node_type::gt_expr:
			return create_binary_ast<::block::gt_expr>(p, matches);
		case pattern::node_type::lte_expr:
			return create_binary_ast<::block::lte_expr>(p, matches);
		case pattern::node_type::gte_expr:
			return create_binary_ast<::block::gte_expr>(p, matches);
		case pattern::node_type::equals_expr:
			return create_binary_ast<::block::equals_expr>(p, matches);
		case pattern::node_type::ne_expr:
			return create_binary_ast<::block::ne_expr>(p, matches);
		case pattern::node_type::mod_expr:
			return create_binary_ast<::block::mod_expr>(p, matches);
		case pattern::node_type::var_expr:
			REPLACER_ASSERT(p->children.size() == 1 || p->name != "", "Var Expr replacer must have a child or reference a capture");
			if (p->children.size()) {
				::block::var_expr::Ptr ve = std::make_shared<::block::var_expr>();
				ve->var1 = ::block::to<::block::var>(create_ast(p->children[0], matches));
			}
			return matches.at(p->name);
		case pattern::node_type::const_expr:
			REPLACER_ASSERT(p->name != "", "Generic const Expr replacer must reference a capture");
			return matches.at(p->name);
		case pattern::node_type::int_const:
			REPLACER_ASSERT(p->has_const || p->name != "", "Int const replacer must have a value or reference a capture");
			if (p->has_const) {
				::block::int_const::Ptr ic = std::make_shared<::block::int_const>();
				ic->value = p->const_val_int;
				return ic;
			}
			return matches.at(p->name);
		case pattern::node_type::double_const:
			REPLACER_ASSERT(p->has_const || p->name != "", "Double const replacer must have a value or reference a capture");
			if (p->has_const) {
				::block::double_const::Ptr dc = std::make_shared<::block::double_const>();
				dc->value = p->const_val_double;
				return dc;
			}
			return matches.at(p->name);
		case pattern::node_type::float_const:
			REPLACER_ASSERT(p->has_const || p->name != "", "Float const replacer must have a value or reference a capture");
			if (p->has_const) {
				::block::float_const::Ptr fc = std::make_shared<::block::float_const>();
				fc->value = p->const_val_double;
				return fc;
			}
			return matches.at(p->name);
		case pattern::node_type::string_const:
			REPLACER_ASSERT(p->has_const || p->name != "", "String const replacer must have a value or reference a capture");
			if (p->has_const) {
				::block::string_const::Ptr sc = std::make_shared<::block::string_const>();
				sc->value = p->const_val_string;
				return sc;
			}
			return matches.at(p->name);
		case pattern::node_type::assign_expr:
			REPLACER_ASSERT(p->children.size() == 2 || p->name != "", "Assign expr replacer must have 2 children or must reference a capture");
			if (p->children.size() == 2) {
				::block::assign_expr::Ptr t = std::make_shared<::block::assign_expr>();
				t->var1 = create_expr_ast(p->children[0], matches);
				t->expr1 = create_expr_ast(p->children[1], matches);
				return t;
			}
			return matches.at(p->name);
		case pattern::node_type::sq_bkt_expr:
			REPLACER_ASSERT(p->children.size() == 2 || p->name != "", "Sq Bkt expr replacer must have 2 children or must reference a capture");
			if (p->children.size() == 2) {
				::block::sq_bkt_expr::Ptr t = std::make_shared<::block::sq_bkt_expr>();
				t->var_expr = create_expr_ast(p->children[0], matches);
				t->index = create_expr_ast(p->children[1], matches);
				return t;
			}
			return matches.at(p->name);
		case pattern::node_type::function_call_expr:	
			REPLACER_ASSERT(p->children.size() > 0 || p->name != "", "Function call must have atleast one child or must reference a capture");
			if (p->children.size() > 0) {
				auto fc = std::make_shared<::block::function_call_expr>();
				fc->expr1 = create_expr_ast(p->children[0], matches);
				for (unsigned i = 1; i < p->children.size(); i++) {
					fc->args.push_back(create_expr_ast(p->children[i], matches));
				}
				return fc;
			}
			return matches.at(p->name);
		case pattern::node_type::initializer_list_expr:
			REPLACER_ASSERT(p->name != "", "Init list expr replacer currently only supports replacing with a reference to a capture");
			return matches.at(p->name);
		case pattern::node_type::member_access_expr:
			REPLACER_ASSERT(p->name != "", "Member access expr replacer currently only supports replacing with a reference to a capture");
			return matches.at(p->name);
		case pattern::node_type::addr_of_expr:
			return create_unary_ast<::block::addr_of_expr>(p, matches);
		case pattern::node_type::var:
			REPLACER_ASSERT(p->name != "" || p->var_name != "", "Var replacer must reference a capture or have a var name");
			if (p->var_name != "") {
				::block::var::Ptr t = std::make_shared<::block::var>();
				t->var_name = p->var_name;
				return t;
			}
			return matches.at(p->name);
		default:
			REPLACER_ASSERT(false, "This replacer is currently not supported");		
			return nullptr;
	}
	return nullptr;
}

void replace_match(block::Ptr ast, match m, pattern::Ptr p) {
	// Before we replace the match with the new AST, first generate the asts 
	// using the new pattern and the captures
	block::Ptr replacement = create_ast(p, m.captures);
	block_replacer replacer;
	replacer.to_replace = m.node;
	replacer.replace_with = replacement;
	ast->accept(&replacer);
}

}
}

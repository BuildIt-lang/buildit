#include "blocks/matchers/matchers.h"
#include "blocks/block.h"
#include "blocks/stmt.h"
#include "blocks/expr.h"
#include "blocks/var.h"
#include <memory>
#include <map>
#include <iostream>
namespace block {
namespace matcher {

static bool pattern_error(bool error, std::string message) {
	if (!error) {
		std::cerr << "PM error: " << message << std::endl;
		return false;
	}
	return true;
}
#define PATTERN_ASSERT(expr, message) if (!pattern_error(expr, message)) return false

template <typename T>
static bool check_unary(std::shared_ptr<pattern> p, block::Ptr node, std::map<std::string, block::Ptr>& captures) {
	if (!isa<T>(node)) return false;
	if (p->children.size() > 0) {
		block::Ptr child = to<::block::unary_expr>(node)->expr1;
		if (!check_match(p->children[0], child, captures))
			return false;
	}
	return true;
}
template <typename T>
static bool check_binary(std::shared_ptr<pattern> p, block::Ptr node, std::map<std::string, block::Ptr>& captures) {
	if (!isa<T>(node)) return false;
	if (p->children.size() > 0) {
		block::Ptr child1 = to<::block::binary_expr>(node)->expr1;
		block::Ptr child2 = to<::block::binary_expr>(node)->expr2;
		if (!check_match(p->children[0], child1, captures)) return false;
		if (!check_match(p->children[1], child2, captures)) return false;
	}
	return true;
}
bool check_match(std::shared_ptr<pattern> p, block::Ptr node, std::map<std::string, block::Ptr>& captures) {
	switch (p->type) {
		case pattern::node_type::block:
			// Every node is a block, nothing to check		
			break;
		case pattern::node_type::expr:
			if (!isa<::block::expr>(node)) return false; 
			break;
		case pattern::node_type::stmt: 
			if (!isa<::block::stmt>(node)) return false; 
			break;
		case pattern::node_type::var:
			// For var we will allow both vars and var_exprs
			if (!isa<::block::var>(node) && !isa<::block::var_expr>(node)) return false;	
			if (p->var_name != "") {
				var::Ptr v;
				if (isa<::block::var>(node))
					v = to<::block::var>(node);
				else
					v = to<::block::var_expr>(node)->var1;
				if (v->var_name != p->var_name) return false;
			}
			break;
		case pattern::node_type::var_expr:
			if (!isa<::block::var_expr>(node)) return false;
			if (p->children.size() > 0) {
				block::Ptr child = to<::block::var_expr>(node)->var1;
				if (!check_match(p->children[0], child, captures))
					return false;
			}
			break;
		case pattern::node_type::unary_expr:
			if (!check_unary<::block::unary_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::binary_expr:
			if (!check_binary<::block::binary_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::not_expr:
			if (!check_unary<::block::not_expr>(p, node, captures)) return false;
			break;	
		case pattern::node_type::unary_minus_expr:
			if (!check_unary<::block::not_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::bitwise_not_expr:
			if (!check_unary<::block::bitwise_not_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::addr_of_expr:
			if (!check_unary<::block::addr_of_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::and_expr:
			if (!check_binary<::block::and_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::bitwise_and_expr:
			if (!check_binary<::block::bitwise_and_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::bitwise_or_expr:
			if (!check_binary<::block::bitwise_or_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::bitwise_xor_expr:
			if (!check_binary<::block::bitwise_xor_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::lshift_expr:
			if (!check_binary<::block::lshift_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::rshift_expr:
			if (!check_binary<::block::rshift_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::or_expr:
			if (!check_binary<::block::or_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::plus_expr:
			if (!check_binary<::block::plus_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::minus_expr:
			if (!check_binary<::block::minus_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::mul_expr:
			if (!check_binary<::block::mul_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::div_expr:
			if (!check_binary<::block::div_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::lt_expr:
			if (!check_binary<::block::lt_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::gt_expr:
			if (!check_binary<::block::gt_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::lte_expr:
			if (!check_binary<::block::lte_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::gte_expr:
			if (!check_binary<::block::gte_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::equals_expr:
			if (!check_binary<::block::equals_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::ne_expr:
			if (!check_binary<::block::ne_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::mod_expr:
			if (!check_binary<::block::mod_expr>(p, node, captures)) return false;
			break;
		case pattern::node_type::assign_expr:
			if (!isa<::block::assign_expr>(node)) return false;
			if (p->children.size() > 0) {
				block::Ptr child1 = to<::block::assign_expr>(node)->var1;
				block::Ptr child2 = to<::block::assign_expr>(node)->expr1;
				if (!check_match(p->children[0], child1, captures)) return false;
				if (!check_match(p->children[1], child2, captures)) return false;
			}
			break;
		case pattern::node_type::const_expr:
			if (!isa<::block::const_expr>(node)) return false;	
			break;
		case pattern::node_type::int_const:
			if (!isa<::block::int_const>(node)) return false;
			if (p->has_const) {
				long long val = to<::block::int_const>(node)->value;
				if (p->const_val_int != val) 
					return false;
			}
			break;
		case pattern::node_type::double_const:
		case pattern::node_type::float_const:
			if (!isa<::block::double_const>(node) && !isa<::block::float_const>(node)) return false;
			if (p->has_const) {
				double val;
				if (isa<::block::double_const>(node))
					val = to<::block::int_const>(node)->value;
				else 
					val = to<::block::float_const>(node)->value;

				if (p->const_val_double != val) 
					return false;
			}
			break;
		case pattern::node_type::string_const:
			if (!isa<::block::string_const>(node)) return false;
			if (p->has_const) {
				std::string val = to<::block::string_const>(node)->value;
				if (p->const_val_string != val) 
					return false;
			}
			break;
		case pattern::node_type::sq_bkt_expr:
			if (!isa<::block::sq_bkt_expr>(node)) return false;
			if (p->children.size() > 0) {
				block::Ptr child1 = to<::block::sq_bkt_expr>(node)->var_expr;
				block::Ptr child2 = to<::block::sq_bkt_expr>(node)->index;
				if (!check_match(p->children[0], child1, captures)) return false;
				if (!check_match(p->children[1], child2, captures)) return false;
			}
			break;
		case pattern::node_type::function_call_expr:
			if (!isa<::block::function_call_expr>(node)) return false;
			if (p->children.size() > 0) {
				auto fc = to<::block::function_call_expr>(node);
				// THere should be an extra expression for the function itself
				if (p->children.size() != fc->args.size() + 1) return false;

				block::Ptr child1 = fc->expr1;
				if (!check_match(p->children[0], child1, captures)) return false;
				for (unsigned i = 0; i < fc->args.size(); i++) {
					if (!check_match(p->children[i+1], fc->args[i], captures)) return false;
				}
			}	
			break;
		case pattern::node_type::initializer_list_expr:
			if (!isa<::block::initializer_list_expr>(node)) return false;
			// No checks for now
			break;
		case pattern::node_type::member_access_expr:
			if (!isa<::block::member_access_expr>(node)) return false;
			break;
		default:
			PATTERN_ASSERT(false, "Node type not implemented in pattern matcher");
	}
	// Now that we have ascertained the structure, check for bindings in captures
	if (p->name != "") {
		// Special matching updates
		if (p->type == pattern::node_type::var && isa<::block::var_expr>(node)) {
			node = to<::block::var_expr>(node)->var1;
		}
	
		if (captures.find(p->name) != captures.end()) {
			// This variable has already had a match, make sure they are the same
			if (!node->is_same(captures[p->name])) 
				return false;
		} else {
			// Update captures
			captures[p->name] = node;
		}
	}

	// Everthing is good, return true
	return true;
}
struct matcher_visitor: public block_visitor {
	std::vector<match> collected_matches;
	std::shared_ptr<pattern> p;
	void try_match(block::Ptr node) {
		std::map<std::string, block::Ptr> captures;
		if (check_match(p, node, captures)) {
			match m;
			m.node = node;
			m.captures = std::move(captures);
			collected_matches.push_back(std::move(m));
		}
	}
	using block_visitor::visit;
	virtual void visit(std::shared_ptr<::block::not_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::unary_minus_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::bitwise_not_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::and_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::bitwise_and_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::bitwise_or_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::bitwise_xor_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::lshift_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::rshift_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::or_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::plus_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::minus_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::mul_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::div_expr> a) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::lt_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::gt_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::lte_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::gte_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::equals_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::ne_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::mod_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::var_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::int_const> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::double_const> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::float_const> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::string_const> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::assign_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::expr_stmt> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::stmt_block> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::decl_stmt> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::if_stmt> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::label> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::label_stmt> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::goto_stmt> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::while_stmt> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::for_stmt> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::break_stmt> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::continue_stmt> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::sq_bkt_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::function_call_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::initializer_list_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}

	virtual void visit(std::shared_ptr<::block::member_access_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::addr_of_expr> a ) {
		block_visitor::visit(a);
		try_match(a);
	}

	virtual void visit(std::shared_ptr<::block::var> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::scalar_type> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::pointer_type> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::function_type> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::array_type> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::builder_var_type> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::named_type> a ) {
		block_visitor::visit(a);
		try_match(a);
	}

	virtual void visit(std::shared_ptr<::block::func_decl> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
	virtual void visit(std::shared_ptr<::block::return_stmt> a ) {
		block_visitor::visit(a);
		try_match(a);
	}
};
std::vector<match> find_all_matches(std::shared_ptr<pattern> p, block::Ptr node) {
	matcher_visitor visitor;
	visitor.p = p;
	node->accept(&visitor);
	return visitor.collected_matches;
}

}
}

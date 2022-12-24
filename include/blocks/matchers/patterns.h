#ifndef BLOCKS_PATTERN_MATCHER_H
#define BLOCKS_PATTERN_MATCHER_H
#include <memory>
#include <string>
#include <vector>
namespace block {
namespace pattern {

struct pattern: public std::enable_shared_from_this<pattern> {
	enum class node_type {
		block,
		expr,
		unary_expr,
		binary_expr,
		not_expr,
		and_expr,
		or_expr,
		plus_expr,
		minus_expr,
		mul_expr,
		div_expr,
		lt_expr,
		gt_expr,
		lte_expr,
		gte_expr,
		equals_expr,
		ne_expr,
		mod_expr,
		var_expr,
		const_expr,
		int_const,
		double_const,
		float_const,
		string_const,
		assign_expr,
		stmt,
		expr_stmt,
		stmt_block,
		decl_stmt,
		if_stmt,
		label,
		label_stmt,
		goto_stmt,
		while_stmt,
		for_stmt,
		break_stmt,
		continue_stmt,
		sq_bkt_expr,
		function_call_expr,
		initializer_list_expr,
		foreign_expr_base,
		member_access_expr,
		addr_of_expr,
		
		var,
		// types will probably be never used in matching, but we will keep them
		type,
		scalar_type,
		pointer_type,
		function_type,
		array_type,
		builder_var_type,
		named_type,

		func_decl,
		return_stmt
	};
	node_type type;
	std::string name;
	std::vector<std::shared_ptr<pattern>> children;

	// Constructor that assigns a name
	template <typename...Args>
	pattern(node_type t, const char* _name, Args...args): children{args...} {
		name = _name;
		type = t;
	}	
	template <typename...Args>
	pattern(node_type t, const std::string& _name, Args...args): children{args...} {
		name = _name;
		type = t;
	}
	// Constructor that doesn't assign a name
	template <typename...Args>
	pattern(node_type t, Args...args): children{args...} {
		name = "";
		type = t;
	}
};
// The helper functions to create nodes

template <typename...Args>
std::shared_ptr<pattern> block(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::block, args...);
}
template <typename...Args>
std::shared_ptr<pattern> expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> unary_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::unary_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> binary_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::binary_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> not_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::not_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> and_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::and_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> or_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::or_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> plus_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::plus_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> minus_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::minus_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> mul_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::mul_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> div_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::div_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> lt_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::lt_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> gt_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::gt_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> lte_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::lte_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> gte_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::gte_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> equals_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::equals_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> ne_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::ne_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> mod_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::mod_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> var_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::var_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> const_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::const_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> int_const(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::int_const, args...);
}
template <typename...Args>
std::shared_ptr<pattern> double_const(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::double_const, args...);
}
template <typename...Args>
std::shared_ptr<pattern> float_const(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::float_const, args...);
}
template <typename...Args>
std::shared_ptr<pattern> string_const(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::string_const, args...);
}
template <typename...Args>
std::shared_ptr<pattern> assign_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::assign_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::stmt, args...);
}
template <typename...Args>
std::shared_ptr<pattern> expr_stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::expr_stmt, args...);
}
template <typename...Args>
std::shared_ptr<pattern> stmt_block(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::stmt_block, args...);
}
template <typename...Args>
std::shared_ptr<pattern> decl_stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::decl_stmt, args...);
}
template <typename...Args>
std::shared_ptr<pattern> if_stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::if_stmt, args...);
}
template <typename...Args>
std::shared_ptr<pattern> label(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::label, args...);
}
template <typename...Args>
std::shared_ptr<pattern> label_stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::label_stmt, args...);
}
template <typename...Args>
std::shared_ptr<pattern> goto_stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::goto_stmt, args...);
}
template <typename...Args>
std::shared_ptr<pattern> while_stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::while_stmt, args...);
}
template <typename...Args>
std::shared_ptr<pattern> for_stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::for_stmt, args...);
}
template <typename...Args>
std::shared_ptr<pattern> break_stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::break_stmt, args...);
}
template <typename...Args>
std::shared_ptr<pattern> continue_stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::continue_stmt, args...);
}
template <typename...Args>
std::shared_ptr<pattern> sq_bkt_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::sq_bkt_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> function_call_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::function_call_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> initializer_list_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::initializer_list_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> foreign_expr_base(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::foreign_expr_base, args...);
}
template <typename...Args>
std::shared_ptr<pattern> member_access_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::member_access_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> addr_of_expr(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::addr_of_expr, args...);
}
template <typename...Args>
std::shared_ptr<pattern> var(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::var, args...);
}
template <typename...Args>
std::shared_ptr<pattern> type(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::type, args...);
}
template <typename...Args>
std::shared_ptr<pattern> scalar_type(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::scalar_type, args...);
}
template <typename...Args>
std::shared_ptr<pattern> pointer_type(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::pointer_type, args...);
}
template <typename...Args>
std::shared_ptr<pattern> function_type(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::function_type, args...);
}
template <typename...Args>
std::shared_ptr<pattern> array_type(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::array_type, args...);
}
template <typename...Args>
std::shared_ptr<pattern> builder_var_type(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::builder_var_type, args...);
}
template <typename...Args>
std::shared_ptr<pattern> named_type(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::named_type, args...);
}
template <typename...Args>
std::shared_ptr<pattern> func_decl(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::func_decl, args...);
}
template <typename...Args>
std::shared_ptr<pattern> return_stmt(Args...args) {
	return std::make_shared<pattern>(pattern::node_type::return_stmt, args...);
}
}
}

#endif

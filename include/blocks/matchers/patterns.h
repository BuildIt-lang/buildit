#ifndef BLOCKS_PATTERN_MATCHER_H
#define BLOCKS_PATTERN_MATCHER_H
#include <memory>
#include <string>
#include <vector>
namespace block {
namespace matcher {

struct pattern: public std::enable_shared_from_this<pattern> {
	typedef std::shared_ptr<pattern> Ptr;
	enum class node_type {
		block,
		expr,
		unary_expr,
		binary_expr,
		not_expr,
		unary_minus_expr,
		bitwise_not_expr,
		and_expr,
		bitwise_and_expr,
		or_expr,
		bitwise_or_expr,
		bitwise_xor_expr,
		plus_expr,
		minus_expr,
		mul_expr,
		div_expr,
		lt_expr,
		gt_expr,
		lte_expr,
		gte_expr,
		lshift_expr,
		rshift_expr,
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
	std::vector<pattern::Ptr> children;
	// Extra fields that can be supplied to be matched
	bool has_const = false;
	double const_val_double = 0;
	int const_val_int = 0;
	std::string const_val_string = "";
	
	// Constructor that assigns a name

	pattern(node_type t, std::string _name, std::vector<pattern::Ptr> _children)
		: type (t), name(_name), children(_children) {}
	pattern(node_type t, std::string _name): type(t), name(_name), children({}) {}

};
// The helper functions to create nodes
static inline pattern::Ptr block(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::block, name);
}
static inline pattern::Ptr expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::expr, name);
}
static inline pattern::Ptr unary_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::unary_expr, name);
}
static inline pattern::Ptr unary_expr(pattern::Ptr x, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::unary_expr, name, std::vector<pattern::Ptr>({x}));
}
static inline pattern::Ptr binary_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::binary_expr, name);
}
static inline pattern::Ptr binary_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::binary_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr not_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::not_expr, name);
}
static inline pattern::Ptr not_expr(pattern::Ptr x, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::not_expr, name, std::vector<pattern::Ptr>({x}));
}
static inline pattern::Ptr operator! (pattern::Ptr x) {
	return not_expr(x);
}
static inline pattern::Ptr and_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::and_expr, name);
}
static inline pattern::Ptr or_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::or_expr, name);
}
static inline pattern::Ptr plus_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::plus_expr, name);
}
static inline pattern::Ptr minus_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::minus_expr, name);
}
static inline pattern::Ptr mul_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::mul_expr, name);
}
static inline pattern::Ptr div_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::div_expr, name);
}
static inline pattern::Ptr lt_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::lt_expr, name);
}
static inline pattern::Ptr gt_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::gt_expr, name);
}
static inline pattern::Ptr lte_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::lte_expr, name);
}
static inline pattern::Ptr gte_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::gte_expr, name);
}
static inline pattern::Ptr equals_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::equals_expr, name);
}
static inline pattern::Ptr ne_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::ne_expr, name);
}
static inline pattern::Ptr mod_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::mod_expr, name);
}
static inline pattern::Ptr and_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::and_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator && (pattern::Ptr x, pattern::Ptr y) {
	return and_expr(x, y);
}
static inline pattern::Ptr or_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::or_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator || (pattern::Ptr x, pattern::Ptr y) {
	return or_expr(x, y);
}
static inline pattern::Ptr plus_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::plus_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator+(pattern::Ptr x, pattern::Ptr y) {
	return plus_expr(x, y);
}
static inline pattern::Ptr minus_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::minus_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator-(pattern::Ptr x, pattern::Ptr y) {
	return minus_expr(x, y);
}
static inline pattern::Ptr mul_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::mul_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator*(pattern::Ptr x, pattern::Ptr y) {
	return mul_expr(x, y);
}
static inline pattern::Ptr div_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::div_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator/ (pattern::Ptr x, pattern::Ptr y) {
	return div_expr(x, y);
}
static inline pattern::Ptr lt_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::lt_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator < (pattern::Ptr x, pattern::Ptr y) {
	return lt_expr(x, y);
}
static inline pattern::Ptr gt_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::gt_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator > (pattern::Ptr x, pattern::Ptr y) {
	return gt_expr(x, y);
}
static inline pattern::Ptr lte_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::lte_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator <= (pattern::Ptr x, pattern::Ptr y) {
	return lte_expr(x, y);
}
static inline pattern::Ptr gte_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::gte_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator >= (pattern::Ptr x, pattern::Ptr y) {
	return gte_expr(x, y);
}
static inline pattern::Ptr equals_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::equals_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator == (pattern::Ptr x, pattern::Ptr y) {
	return equals_expr(x, y);
}
static inline pattern::Ptr ne_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::ne_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator != (pattern::Ptr x, pattern::Ptr y) {
	return ne_expr(x, y);
}
static inline pattern::Ptr mod_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::mod_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator % (pattern::Ptr x, pattern::Ptr y) {
	return mod_expr(x, y);
}
static inline pattern::Ptr assign_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::assign_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr sq_bkt_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::sq_bkt_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr var_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::var_expr, name);
}
static inline pattern::Ptr const_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::const_expr, name);
}

// Lets not mess with overloads on constants because they can mess with pointer arithmetic
static inline pattern::Ptr int_const(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::int_const, name);
}
static inline pattern::Ptr int_const(const int val, std::string name = "") {
	auto p = std::make_shared<pattern>(pattern::node_type::int_const, name);
	p->has_const = true;
	p->const_val_int = val;
	return p;
}
static inline pattern::Ptr double_const(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::double_const, name);
}
static inline pattern::Ptr double_const(const double val, std::string name = "") {
	auto p = std::make_shared<pattern>(pattern::node_type::double_const, name);
	p->has_const = true;
	p->const_val_double = val;
	return p;
}
static inline pattern::Ptr float_const(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::float_const, name);
}
static inline pattern::Ptr float_const(const double val, std::string name = "") {
	auto p = std::make_shared<pattern>(pattern::node_type::float_const, name);
	p->has_const = true;
	p->const_val_double = val;
	return p;
}
static inline pattern::Ptr string_const(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::string_const, name);
}
// string_const constructor needs a special name so that the value doesn't mix up with names
static inline pattern::Ptr string_const_with_val(std::string val, std::string name = "") {
	auto p = std::make_shared<pattern>(pattern::node_type::string_const, name);
	p->has_const = true;
	p->const_val_string = val;
	return p;
}
static inline pattern::Ptr assign_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::assign_expr, name);
}

static inline pattern::Ptr unary_minus_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::unary_minus_expr, name);
}
static inline pattern::Ptr unary_minus_expr(pattern::Ptr x, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::unary_minus_expr, name, std::vector<pattern::Ptr>({x}));
}
static inline pattern::Ptr operator-(pattern::Ptr x) {
	return unary_minus_expr(x);
}
static inline pattern::Ptr bitwise_not_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::bitwise_not_expr, name);
}
static inline pattern::Ptr bitwise_not_expr(pattern::Ptr x, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::bitwise_not_expr, name, std::vector<pattern::Ptr>({x}));
}
static inline pattern::Ptr operator~(pattern::Ptr x) {
	return bitwise_not_expr(x);
}
static inline pattern::Ptr bitwise_and_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::bitwise_and_expr, name);
}
static inline pattern::Ptr bitwise_and_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::bitwise_and_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator & (pattern::Ptr x, pattern::Ptr y) {
	return bitwise_and_expr(x, y);
}
static inline pattern::Ptr bitwise_or_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::bitwise_or_expr, name);
}
static inline pattern::Ptr bitwise_or_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::bitwise_or_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator | (pattern::Ptr x, pattern::Ptr y) {
	return bitwise_or_expr(x, y);
}
static inline pattern::Ptr bitwise_xor_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::bitwise_xor_expr, name);
}
static inline pattern::Ptr bitwise_xor_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::bitwise_xor_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator ^ (pattern::Ptr x, pattern::Ptr y) {
	return bitwise_xor_expr(x, y);
}
static inline pattern::Ptr lshift_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::lshift_expr, name);
}
static inline pattern::Ptr lshift_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::lshift_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator << (pattern::Ptr x, pattern::Ptr y) {
	return lshift_expr(x, y);
}
static inline pattern::Ptr rshift_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::rshift_expr, name);
}
static inline pattern::Ptr rshift_expr(pattern::Ptr x, pattern::Ptr y, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::rshift_expr, name, std::vector<pattern::Ptr>({x, y}));
}
static inline pattern::Ptr operator >> (pattern::Ptr x, pattern::Ptr y) {
	return rshift_expr(x, y);
}

static inline pattern::Ptr stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::stmt, name);
}
static inline pattern::Ptr expr_stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::expr_stmt, name);
}
static inline pattern::Ptr stmt_block(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::stmt_block, name);
}
static inline pattern::Ptr decl_stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::decl_stmt, name);
}
static inline pattern::Ptr if_stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::if_stmt, name);
}
static inline pattern::Ptr label(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::label, name);
}
static inline pattern::Ptr label_stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::label_stmt, name);
}
static inline pattern::Ptr goto_stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::goto_stmt, name);
}
static inline pattern::Ptr while_stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::while_stmt, name);
}
static inline pattern::Ptr for_stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::for_stmt, name);
}
static inline pattern::Ptr break_stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::break_stmt, name);
}
static inline pattern::Ptr continue_stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::continue_stmt, name);
}
static inline pattern::Ptr sq_bkt_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::sq_bkt_expr, name);
}
static inline pattern::Ptr function_call_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::function_call_expr, name);
}
static inline pattern::Ptr initializer_list_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::initializer_list_expr, name);
}
static inline pattern::Ptr foreign_expr_base(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::foreign_expr_base, name);
}
static inline pattern::Ptr member_access_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::member_access_expr, name);
}
static inline pattern::Ptr addr_of_expr(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::addr_of_expr, name);
}
static inline pattern::Ptr addr_of_expr(pattern::Ptr x, std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::addr_of_expr, name, std::vector<pattern::Ptr>({x}));
}
static inline pattern::Ptr var(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::var, name);
}
static inline pattern::Ptr type(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::type, name);
}
static inline pattern::Ptr scalar_type(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::scalar_type, name);
}
static inline pattern::Ptr pointer_type(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::pointer_type, name);
}
static inline pattern::Ptr function_type(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::function_type, name);
}
static inline pattern::Ptr array_type(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::array_type, name);
}
static inline pattern::Ptr builder_var_type(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::builder_var_type, name);
}
static inline pattern::Ptr named_type(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::named_type, name);
}
static inline pattern::Ptr func_decl(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::func_decl, name);
}
static inline pattern::Ptr return_stmt(std::string name = "") {
	return std::make_shared<pattern>(pattern::node_type::return_stmt, name);
}



} // namespace matchers
} // namespace block

#endif

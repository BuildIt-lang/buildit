#ifndef C_CODE_GENERATOR_H
#define C_CODE_GENERATOR_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"
#include "util/printer.h"
#include <unordered_map>
#include <unordered_set>

namespace block {
class c_code_generator : public block_visitor {
private:
	void emit_binary_expr(binary_expr::Ptr, std::string);

public:
	using block_visitor::visit;
	c_code_generator(std::ostream &_oss) : oss(_oss) {}
	std::ostream &oss;
	int curr_indent = 0;
	bool decl_only = false;
	virtual void visit(not_expr::Ptr);
	virtual void visit(and_expr::Ptr);
	virtual void visit(bitwise_and_expr::Ptr);
	virtual void visit(or_expr::Ptr);
	virtual void visit(bitwise_or_expr::Ptr);
	virtual void visit(plus_expr::Ptr);
	virtual void visit(minus_expr::Ptr);
	virtual void visit(mul_expr::Ptr);
	virtual void visit(div_expr::Ptr);
	virtual void visit(lt_expr::Ptr);
	virtual void visit(gt_expr::Ptr);
	virtual void visit(lte_expr::Ptr);
	virtual void visit(gte_expr::Ptr);
	virtual void visit(lshift_expr::Ptr);
	virtual void visit(rshift_expr::Ptr);
	virtual void visit(equals_expr::Ptr);
	virtual void visit(ne_expr::Ptr);
	virtual void visit(mod_expr::Ptr);
	virtual void visit(var_expr::Ptr);
	virtual void visit(int_const::Ptr);
	virtual void visit(double_const::Ptr);
	virtual void visit(float_const::Ptr);
	virtual void visit(string_const::Ptr);
	virtual void visit(assign_expr::Ptr);
	virtual void visit(expr_stmt::Ptr);
	virtual void visit(stmt_block::Ptr);
	virtual void visit(decl_stmt::Ptr);
	virtual void visit(if_stmt::Ptr);
	virtual void visit(while_stmt::Ptr);
	virtual void visit(for_stmt::Ptr);
	virtual void visit(break_stmt::Ptr);
	virtual void visit(continue_stmt::Ptr);
	virtual void visit(sq_bkt_expr::Ptr);
	virtual void visit(function_call_expr::Ptr);
	virtual void visit(initializer_list_expr::Ptr);

	virtual void visit(var::Ptr);
	virtual void visit(scalar_type::Ptr);
	virtual void visit(pointer_type::Ptr);
	virtual void visit(array_type::Ptr);
	virtual void visit(builder_var_type::Ptr);
	virtual void visit(named_type::Ptr);

	void handle_func_arg(var::Ptr a);
	virtual void visit(func_decl::Ptr);
	virtual void visit(return_stmt::Ptr);
	virtual void visit(member_access_expr::Ptr);
	virtual void visit(addr_of_expr::Ptr);

	virtual void visit(goto_stmt::Ptr);
	virtual void visit(label_stmt::Ptr);
	

	static void generate_code(block::Ptr ast, std::ostream &oss, int indent = 0, bool decl_only = false) {
		c_code_generator generator(oss);
		generator.decl_only = decl_only;
		generator.curr_indent = indent;
		ast->accept(&generator);
		oss << std::endl;
	}
};
} // namespace block
#endif

#ifndef C_CODE_GENERATOR_H
#define C_CODE_GENERATOR_H
#include "blocks/block_visitor.h"
#include "blocks/stmt.h"
#include "builder/dyn_var.h"
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
	virtual void visit(reference_type::Ptr);
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
	template <typename T>
	static void generate_struct_decl(std::ostream &oss, int indent = 0) {
		static_assert(std::is_base_of<builder::var, T>::value, "Template argument should be a dyn_var");
		auto save = builder::options::track_members;
		builder::options::track_members = true;
		T v = builder::with_name("_");
		builder::options::track_members = save;
		/* Dump the type */
		c_code_generator generator(oss);
		printer::indent(oss, indent);
		auto var_type = T::create_block_type();
		assert(isa<named_type>(var_type) && "Cannot create struct declarations for un-named types");
		assert(to<named_type>(var_type)->template_args.size() == 0 &&
		       "Cannot yet, generate decls for types with template args");
		oss << "struct " << to<named_type>(var_type)->type_name << " {\n";
		indent++;

		for (auto member : v.members) {
			printer::indent(oss, indent);
			auto decl = std::make_shared<decl_stmt>();
			decl->decl_var = member->block_var;
			decl->accept(&generator);
			oss << std::endl;
		}
		indent--;
		printer::indent(oss, indent);
		oss << "};" << std::endl;
	}
};
} // namespace block
#endif

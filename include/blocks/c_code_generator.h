#ifndef C_CODE_GENERATOR_H
#define C_CODE_GENERATOR_H

#ifdef ENABLE_D2X
#include "d2x/d2x.h"
#include "d2x/utils.h"
#endif

#include "blocks/block_visitor.h"
#include "blocks/stmt.h"
#include "util/printer.h"
#include <unordered_map>
#include <unordered_set>
#include "builder/forward_declarations.h"
#include "builder/providers_dyn_var.h"

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
	bool use_d2x = false;

#ifdef ENABLE_D2X
	d2x::d2x_context xctx;
#endif
	void save_static_info(block::Ptr a);
	void nextl(void);

	virtual void visit(not_expr::Ptr) override;
	virtual void visit(unary_minus_expr::Ptr) override;
	virtual void visit(bitwise_not_expr::Ptr) override;
	virtual void visit(and_expr::Ptr) override;
	virtual void visit(bitwise_and_expr::Ptr) override;
	virtual void visit(or_expr::Ptr) override;
	virtual void visit(bitwise_or_expr::Ptr) override;
	virtual void visit(bitwise_xor_expr::Ptr) override;
	virtual void visit(plus_expr::Ptr) override;
	virtual void visit(minus_expr::Ptr) override;
	virtual void visit(mul_expr::Ptr) override;
	virtual void visit(div_expr::Ptr) override;
	virtual void visit(lt_expr::Ptr) override;
	virtual void visit(gt_expr::Ptr) override;
	virtual void visit(lte_expr::Ptr) override;
	virtual void visit(gte_expr::Ptr) override;
	virtual void visit(lshift_expr::Ptr) override;
	virtual void visit(rshift_expr::Ptr) override;
	virtual void visit(equals_expr::Ptr) override;
	virtual void visit(ne_expr::Ptr) override;
	virtual void visit(mod_expr::Ptr) override;
	virtual void visit(var_expr::Ptr) override;
	virtual void visit(int_const::Ptr) override;
	virtual void visit(double_const::Ptr) override;
	virtual void visit(float_const::Ptr) override;
	virtual void visit(string_const::Ptr) override;
	virtual void visit(assign_expr::Ptr) override;
	virtual void visit(expr_stmt::Ptr) override;
	virtual void visit(stmt_block::Ptr) override;
	virtual void visit(decl_stmt::Ptr) override;
	virtual void visit(if_stmt::Ptr) override;
	virtual void visit(switch_stmt::Ptr) override;
	virtual void visit(case_stmt::Ptr) override;
	virtual void visit(while_stmt::Ptr) override;
	virtual void visit(for_stmt::Ptr) override;
	virtual void visit(break_stmt::Ptr) override;
	virtual void visit(continue_stmt::Ptr) override;
	virtual void visit(sq_bkt_expr::Ptr) override;
	virtual void visit(function_call_expr::Ptr) override;
	virtual void visit(initializer_list_expr::Ptr) override;

	virtual void visit(var::Ptr) override;
	virtual void visit(scalar_type::Ptr) override;
	virtual void visit(pointer_type::Ptr) override;
	virtual void visit(reference_type::Ptr) override;
	virtual void visit(array_type::Ptr) override;
	virtual void visit(builder_var_type::Ptr) override;
	virtual void visit(named_type::Ptr) override;
	virtual void visit(anonymous_type::Ptr) override;

	virtual void visit(func_decl::Ptr) override;
	virtual void visit(struct_decl::Ptr) override;
	virtual void visit(return_stmt::Ptr) override;
	virtual void visit(member_access_expr::Ptr) override;
	virtual void visit(addr_of_expr::Ptr) override;
	virtual void visit(cast_expr::Ptr) override;

	virtual void visit(goto_stmt::Ptr) override;
	virtual void visit(label_stmt::Ptr) override;

	void print_pragma(stmt::Ptr);
	void print_struct_type(struct_decl::Ptr a);
	void handle_child(expr::Ptr parent, expr::Ptr child, bool is_left);

	static void generate_code(block::Ptr ast, std::ostream &oss, int indent = 0, bool decl_only = false) {
		c_code_generator generator(oss);
		generator.decl_only = decl_only;
		generator.curr_indent = indent;
		ast->accept(&generator);
		oss << std::endl;
	}
	static void generate_code_d2x(block::Ptr ast, std::ostream &oss, int indent = 0, bool decl_only = false) {
#ifndef ENABLE_D2X
		assert(false && "Cannot generate code with D2X support without ENABLE_D2X build option");
#endif
		c_code_generator generator(oss);
		generator.use_d2x = true;
		generator.decl_only = decl_only;
		generator.curr_indent = indent;
		ast->accept(&generator);
		oss << std::endl;
	}
	template <typename T>
	static void generate_struct_decl(std::ostream &oss, int indent = 0) {
		static_assert(builder::is_dyn_var_type<T>::value, "Template argument should be a dyn_var");
		auto save = builder::user_defined_provider_track_members;
		builder::user_defined_provider_track_members = true;
		T v = builder::with_name("_");
		builder::user_defined_provider_track_members = save;

		// Construct a struct decl
		auto sd = std::make_shared<struct_decl>();
		auto var_type = T::create_block_type();
		assert(isa<named_type>(var_type) && "Cannot create struct declarations for un-named types");
		assert(to<named_type>(var_type)->template_args.size() == 0 &&
		       "Cannot yet, generate decls for types with template args");

		sd->struct_name = to<named_type>(var_type)->type_name;
		for (auto member: v.user_defined_members) {
			auto decl = std::make_shared<decl_stmt>();
			decl->decl_var = member;
			decl->init_expr = nullptr;
			sd->members.push_back(decl);
		}

		/* Dump the type */
		c_code_generator generator(oss);
		sd->accept(&generator);
		oss << std::endl;
	}
};
} // namespace block
#endif

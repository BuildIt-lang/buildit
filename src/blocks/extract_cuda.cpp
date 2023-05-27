#include "blocks/extract_cuda.h"
#include "blocks/c_code_generator.h"
#include "builder/dyn_var.h"
#include <sstream>

namespace block {
// Unique counter to name generated kernels
int total_created_kernels = 0;

// Local utility visitor to find the statement block
// a particular statement is from
struct parent_finder : public block_visitor {
public:
	using block_visitor::visit;
	stmt::Ptr to_find;
	stmt_block::Ptr parent_found = nullptr;
	virtual void visit(stmt_block::Ptr block) {
		block_visitor::visit(block);
		for (auto stmt : block->stmts) {
			if (to_find == stmt) {
				parent_found = block;
			}
		}
	}
};

static std::vector<var::Ptr> extract_extern_vars(block::Ptr function, stmt::Ptr from, var::Ptr outer, var::Ptr inner) {
	// This visitor finds the list of the variables that are declared inside the function
	// This is done to ignore the globals
	// Not being used right now, we want to hoist the globals too
	gather_declared_vars func_dec;
	function->accept(&func_dec);

	// This identifies the variables that are declared inside the block
	gather_declared_vars dec;
	from->accept(&dec);

	// This finally identifies the variables that are used inside the loop nest, declared in the function
	// But not declared inside the block
	gather_extern_vars exts;
	exts.declared = dec.declared;
	exts.declared.push_back(outer);
	exts.declared.push_back(inner);
	// exts.func_declared = func_dec.declared;

	from->accept(&exts);

	return exts.gathered;
}

static void var_replace_all(stmt::Ptr body, var::Ptr from, var::Ptr to);
block::Ptr extract_single_cuda(block::Ptr from, std::vector<decl_stmt::Ptr> &new_decls) {
	if (!isa<stmt_block>(from)) {
		std::cerr << "extract_single_cuda() expects a stmt_block" << std::endl;
		return nullptr;
	}

	int is_coop = 0;
	stmt::Ptr found_loop = annotation_finder::find_annotation(from, CUDA_KERNEL);
	if (found_loop == nullptr) {
		found_loop = annotation_finder::find_annotation(from, CUDA_KERNEL_COOP);
		if (found_loop == nullptr) {
			return nullptr;
		}
		is_coop = 1;
	}

	// First we assert that the stmt we have found is a for loop
	assert(isa<for_stmt>(found_loop) && "The " CUDA_KERNEL " annotation should be only applied to a for loop");
	// We will also assert that this for loop has a single other for loop
	for_stmt::Ptr outer_loop = to<for_stmt>(found_loop);
	assert(isa<stmt_block>(outer_loop->body) && to<stmt_block>(outer_loop->body)->stmts.size() == 1 &&
	       isa<for_stmt>(to<stmt_block>(outer_loop->body)->stmts[0]) && "Loops for device should be doubly nested");
	for_stmt::Ptr inner_loop = to<for_stmt>(to<stmt_block>(outer_loop->body)->stmts[0]);

	var::Ptr outer_var;
	if (isa<decl_stmt>(outer_loop->decl_stmt)) {
		outer_var = to<decl_stmt>(outer_loop->decl_stmt)->decl_var;
	} else {
		// If this is not a decl, it must be an assign
		outer_var = to<var_expr>(to<assign_expr>(to<expr_stmt>(outer_loop->decl_stmt)->expr1)->var1)->var1;
	}
	var::Ptr inner_var;
	if (isa<decl_stmt>(inner_loop->decl_stmt)) {
		inner_var = to<decl_stmt>(inner_loop->decl_stmt)->decl_var;
	} else {
		// If this is not a decl, it must be an assign
		inner_var = to<var_expr>(to<assign_expr>(to<expr_stmt>(inner_loop->decl_stmt)->expr1)->var1)->var1;
	}
	std::vector<var::Ptr> vars = extract_extern_vars(from, inner_loop->body, outer_var, inner_var);

	int this_kern_index = total_created_kernels;
	total_created_kernels++;
	std::vector<var::Ptr> ret_vars;
	if (is_coop) {
		// If this is coop, we will create some extra decls to return the copied values
		int i = 0;
		for (auto v : vars) {
			std::ostringstream type_str;
			c_code_generator::generate_code(v->var_type, type_str, 0);
			std::string type_string = type_str.str();
			type_string.pop_back();
			auto v_new = std::make_shared<var>();
			v_new->var_type = builder::dyn_var<char>::create_block_type();
			// v_new->var_type = v->var_type;
			v_new->var_name = "ret_" + std::to_string(this_kern_index) + "_" + std::to_string(i) +
					  "[sizeof(" + type_string + ")]";
			auto v_new_ret = std::make_shared<var>();
			v_new_ret->var_type = builder::dyn_var<char>::create_block_type();
			v_new_ret->var_name = "ret_" + std::to_string(this_kern_index) + "_" + std::to_string(i);
			ret_vars.push_back(v_new_ret);
			i++;
			v_new->setMetadata<std::vector<std::string>>("attributes", {"__device__"});

			auto decl_new = std::make_shared<decl_stmt>();
			decl_new->decl_var = v_new;
			decl_new->init_expr = nullptr;
			new_decls.push_back(decl_new);
		}
	}

	assert(isa<lt_expr>(outer_loop->cond) && "CUDA loops should have condition of the form < ...");
	assert(isa<lt_expr>(inner_loop->cond) && "CUDA loops should have condition of the form < ...");

	expr::Ptr cta_count = to<lt_expr>(outer_loop->cond)->expr2;
	expr::Ptr thread_count = to<lt_expr>(inner_loop->cond)->expr2;

	var::Ptr cta_id = std::make_shared<var>();
	cta_id->var_name = "blockIdx.x";
	cta_id->var_type = builder::dyn_var<int>::create_block_type();

	var::Ptr thread_id = std::make_shared<var>();
	thread_id->var_name = "threadIdx.x";
	thread_id->var_type = builder::dyn_var<int>::create_block_type();

	var_replace_all(inner_loop->body, outer_var, cta_id);
	var_replace_all(inner_loop->body, inner_var, thread_id);

	func_decl::Ptr kernel = std::make_shared<func_decl>();
	kernel->func_name = "cuda_kernel_" + std::to_string(this_kern_index);

	kernel->return_type = builder::dyn_var<void>::create_block_type();

	function_call_expr::Ptr call = std::make_shared<function_call_expr>();
	var::Ptr call_name = std::make_shared<var>();
	if (!is_coop) {
		call_name->var_type = builder::dyn_var<int>::create_block_type();
		call_name->var_name = kernel->func_name;
		call_name->var_name += "<<<";

		std::ostringstream cta_count_str, thread_count_str;
		c_code_generator::generate_code(cta_count, cta_count_str, 0);
		c_code_generator::generate_code(thread_count, thread_count_str, 0);

		call_name->var_name += cta_count_str.str();
		// There is new line at the end - always
		call_name->var_name.pop_back();
		call_name->var_name += ", " + thread_count_str.str();
		call_name->var_name.pop_back();
		call_name->var_name += ">>>";

	} else {
		call_name->var_type = builder::dyn_var<int>::create_block_type();
		call_name->var_name = "runtime::LaunchCooperativeKernel";
		var_expr::Ptr param1 = std::make_shared<var_expr>();
		var::Ptr param1_var = std::make_shared<var>();
		param1_var->var_type = builder::dyn_var<int>::create_block_type();
		param1_var->var_name = "(void*)" + kernel->func_name;
		param1->var1 = param1_var;
		call->args.push_back(param1);
		call->args.push_back(cta_count);
		call->args.push_back(thread_count);
	}
	var_expr::Ptr call_var_expr = std::make_shared<var_expr>();
	call_var_expr->var1 = call_name;
	call->expr1 = call_var_expr;
	expr_stmt::Ptr call_stmt = std::make_shared<expr_stmt>();
	call_stmt->expr1 = call;

	function_call_expr::Ptr call_sync = std::make_shared<function_call_expr>();
	var::Ptr call_name_sync = std::make_shared<var>();
	call_name_sync->var_type = builder::dyn_var<int>::create_block_type();
	call_name_sync->var_name = "cudaDeviceSynchronize";
	var_expr::Ptr call_var_expr_sync = std::make_shared<var_expr>();
	call_var_expr_sync->var1 = call_name_sync;
	call_sync->expr1 = call_var_expr_sync;
	expr_stmt::Ptr call_stmt_sync = std::make_shared<expr_stmt>();
	call_stmt_sync->expr1 = call_sync;

	for (unsigned int i = 0; i < vars.size(); i++) {
		std::string arg_name = "arg" + std::to_string(i);
		var::Ptr arg = std::make_shared<var>();
		arg->var_name = arg_name;
		arg->var_type = vars[i]->var_type;
		var_replace_all(inner_loop->body, vars[i], arg);
		kernel->args.push_back(arg);
		var_expr::Ptr arg_expr = std::make_shared<var_expr>();
		arg_expr->var1 = vars[i];
		call->args.push_back(arg_expr);
	}
	stmt_block::Ptr new_stmts = std::make_shared<stmt_block>();

	// block::stmt_block::Ptr old_stmts = to<block::stmt_block>(from);
	parent_finder finder;
	finder.to_find = found_loop;
	from->accept(&finder);
	stmt_block::Ptr old_stmts = finder.parent_found;

	kernel->body = inner_loop->body;

	// If this is a coop kernel, return the values
	std::vector<stmt::Ptr> copy_backs;
	if (is_coop) {
		auto if_s = std::make_shared<if_stmt>();
		auto nvar = std::make_shared<var>();
		nvar->var_type = builder::dyn_var<int>::create_block_type();
		nvar->var_name = "!(blockIdx.x * blockDim.x + threadIdx.x)";
		auto nvar_expr = std::make_shared<var_expr>();
		nvar_expr->var1 = nvar;
		if_s->cond = nvar_expr;
		if_s->then_stmt = std::make_shared<stmt_block>();
		if_s->else_stmt = std::make_shared<stmt_block>();
		// Add an assignment for each variable
		int i = 0;
		auto v_copy = std::make_shared<var>();
		v_copy->var_type = builder::dyn_var<void(void)>::create_block_type();
		v_copy->var_name = "runtime::cudaMemcpyFromSymbolMagic";
		auto v_copy_expr = std::make_shared<var_expr>();
		v_copy_expr->var1 = v_copy;

		auto m_copy = std::make_shared<var>();
		m_copy->var_type = builder::dyn_var<void(void)>::create_block_type();
		m_copy->var_name = "runtime::cudaMemcpyToSymbolMagic";
		auto m_copy_expr = std::make_shared<var_expr>();
		m_copy_expr->var1 = m_copy;

		for (auto v : kernel->args) {
			auto rhs = std::make_shared<var_expr>();
			rhs->var1 = v;
			auto lhs = std::make_shared<var_expr>();
			lhs->var1 = ret_vars[i];
			auto f2 = std::make_shared<function_call_expr>();
			f2->expr1 = m_copy_expr;
			f2->args.push_back(lhs);
			f2->args.push_back(rhs);

			auto nexpr_stmt = std::make_shared<expr_stmt>();
			nexpr_stmt->expr1 = f2;
			to<stmt_block>(if_s->then_stmt)->stmts.push_back(nexpr_stmt);

			// Also create a copy back
			auto f = std::make_shared<function_call_expr>();
			f->expr1 = v_copy_expr;
			auto addr = std::make_shared<addr_of_expr>();
			addr->expr1 = call->args[i + 3];
			f->args.push_back(addr);
			f->args.push_back(lhs);

			auto stmt = std::make_shared<expr_stmt>();
			stmt->expr1 = f;
			copy_backs.push_back(stmt);

			i++;
		}
		to<stmt_block>(kernel->body)->stmts.push_back(if_s);
	}

	for (auto stmt : old_stmts->stmts) {
		if (stmt != found_loop)
			new_stmts->stmts.push_back(stmt);
		else {
			new_stmts->stmts.push_back(call_stmt);
			new_stmts->stmts.push_back(call_stmt_sync);
			for (auto a : copy_backs)
				new_stmts->stmts.push_back(a);
		}
	}

	old_stmts->stmts = new_stmts->stmts;

	std::vector<std::string> attrs;
	attrs.push_back("__global__");

	kernel->setMetadata("attributes", attrs);

	return kernel;
}

void cuda_var_replacer::visit(var_expr::Ptr expr) {
	if (expr->var1 == to_replace)
		expr->var1 = replace_with;
}
static void var_replace_all(stmt::Ptr body, var::Ptr from, var::Ptr to) {
	cuda_var_replacer replacer;
	replacer.to_replace = from;
	replacer.replace_with = to;
	body->accept(&replacer);
}

void gather_declared_vars::visit(decl_stmt::Ptr stmt) {
	var::Ptr var1 = stmt->decl_var;
	if (std::find(declared.begin(), declared.end(), var1) == declared.end())
		declared.push_back(var1);
}

void gather_declared_vars::visit(func_decl::Ptr stmt) {
	stmt->return_type->accept(this);
	for (auto arg : stmt->args) {
		if (std::find(declared.begin(), declared.end(), arg) == declared.end())
			declared.push_back(arg);
	}
	stmt->body->accept(this);
}

void gather_extern_vars::visit(var_expr::Ptr expr) {
	var::Ptr var1 = expr->var1;
	// We want to ignore all the variables with function types
	if (isa<function_type>(var1->var_type))
		return;
	if (std::find(gathered.begin(), gathered.end(), var1) == gathered.end() &&
	    std::find(declared.begin(), declared.end(), var1) == declared.end())
		gathered.push_back(var1);
}

std::vector<block::Ptr> extract_cuda_from(block::Ptr from) {
	std::vector<block::Ptr> new_decls;
	block::Ptr kernel = nullptr;
	std::vector<decl_stmt::Ptr> new_var_decls;
	while ((kernel = extract_single_cuda(from, new_var_decls))) {
		for (auto a : new_var_decls)
			new_decls.push_back(a);
		new_var_decls.clear();
		new_decls.push_back(kernel);
	}
	return new_decls;
}

} // namespace block

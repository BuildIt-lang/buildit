#ifndef SIGNATURE_EXTRACT_H
#define SIGNATURE_EXTRACT_H

#include "builder/run_states.h"
#include <tuple>

namespace builder {


// extract_signature impl is a struct because we want to
// write partial specialization

/*
ProcessedArgTypes - A tuple of types from the function type that have already been processed
RemainingArgTypes - A tuple of types from the funciton type that are yet to be processed

// Template arguments on the static members are for everything that can be implicitly inferred
F - invocable type - can be function or lambda
NextParam- the very next params the user provided, may or may not be used just yet
OtherParams... - all the remaining params the other 


ProcessedArgTypes is borrowed from above and hence is not really a pack
*/

// Helper function to create a return statement
template <typename T>
void create_return_stmt(const dyn_var<T>& a) {
	auto e = to_expr(a);
	get_run_state()->remove_node_from_sequence(e);
	get_run_state()->commit_uncommitted();
	if (get_run_state()->is_catching_up())
		return;
	block::return_stmt::Ptr ret_stmt = std::make_shared<block::return_stmt>();
	// ret_stmt->static_offset = a.block_expr->static_offset;
	//  Finding conflicts between return statements is somehow really hard.
	//  So treat each return statement as different. This is okay, because a
	//  jump is as bad a return. Also no performance issues
	ret_stmt->static_offset = tracer::get_unique_tag();
	ret_stmt->return_val = e;
	get_run_state()->add_stmt_to_current_block(ret_stmt, true);
}

template <typename ProcessedArgTypes, typename RemainingArgTypes, typename ReturnType, typename Enable=void>
struct extract_signature_impl {

	// The ProcessedArgType is used as it is here, but in an actual implementation it will be an 
	// expansion from inside the tuple
	template <typename F, typename NextParam, typename...OtherParams>
	static void fill_invocation(invocation_state* i_state, F func, int arg_index, 
		ProcessedArgTypes processed_arg, NextParam&& next_param, OtherParams&&...other_params);
};

// For partial specializations, the template parameter is a pack but is used inside a tuple
// 1. When the next argument to process is dyn_var  - It has to be dyn_var<T> not any derived types
template <typename...ProcessedArgTypes, typename NextArgWrap, typename...RemainingArgTypes, typename ReturnType>
struct extract_signature_impl<std::tuple<ProcessedArgTypes...>, std::tuple<NextArgWrap, RemainingArgTypes...>, ReturnType, typename std::enable_if<is_dyn_var_type<NextArgWrap>::value>::type> {
	using NextArg = typename NextArgWrap::stored_type;
	// The dyn_var version doesn't need any NextParam to exist or be used
	template <typename F, typename...OtherParams>
	static void fill_invocation(invocation_state* i_state, F func, int arg_index, ProcessedArgTypes...processed_args, OtherParams&&...other_params) {
		// We don't need to heap allocate objects anymore, just throw in a with_block_var, it has all
		// the information to create the dyn_var when invoking the actual funciton
		std::string arg_name = "arg" + std::to_string(arg_index);
		auto var = std::make_shared<block::var>();	
		var->var_name = arg_name;
		var->var_type = dyn_var<NextArg>::create_block_type();
		var->var_type->static_offset = var->static_offset = tracer::get_unique_tag();
		i_state->generated_func_decl->args.push_back(var);
		// We explain below by with_block_var is wrapped in a tuple
		extract_signature_impl<std::tuple<ProcessedArgTypes..., std::tuple<with_block_var>>, std::tuple<RemainingArgTypes...>, ReturnType>::fill_invocation(i_state, func, arg_index + 1, std::forward<ProcessedArgTypes>(processed_args)..., std::tuple<with_block_var>(with_block_var(var)), std::forward<OtherParams>(other_params)...);
	}
};

// 2. When the next argumetn to process isn't a dyn_var
template <typename...ProcessedArgTypes, typename NextArg, typename...RemainingArgTypes, typename ReturnType>
struct extract_signature_impl<std::tuple<ProcessedArgTypes...>, std::tuple<NextArg, RemainingArgTypes...>, ReturnType, typename std::enable_if<!is_dyn_var_type<NextArg>::value>::type> {
	// This version needs a NextParam
	template <typename F, typename NextParam, typename...OtherParams>
	static void fill_invocation(invocation_state* i_state, F func, int arg_index, ProcessedArgTypes...processed_args, NextParam&& next_param, OtherParams&&...other_params) {
		// No with name this time, just throw in the arg into processed args
		// Pass it as NextParam type, not NextArg type. Defer all conversions till the final call
		// We wrap the NextParam type in a tuple so that types that are references so that the arguments can be captured by 
		// value in the lambda. This way references remain references. If we passed the reference along directly, it becomes a copy
		// We also don't wrap NextParam&& because rvalue references cannot be wrapped in a tuple (if it needs to be copyable)
		extract_signature_impl<std::tuple<ProcessedArgTypes..., std::tuple<NextParam>>, std::tuple<RemainingArgTypes...>, ReturnType>::fill_invocation(i_state, func, arg_index, std::forward<ProcessedArgTypes>(processed_args)..., std::tuple<NextParam>(std::forward<NextParam>(next_param)), std::forward<OtherParams>(other_params)...);
	}
};

// 3. When there aren't any more args to fill AND the return type is void
// No remaining types now
template <typename...ProcessedArgTypes>
struct extract_signature_impl<std::tuple<ProcessedArgTypes...>, std::tuple<>, void, void> {

	// Take other_args but discard them
	template <typename F, typename...OtherParams>
	static void fill_invocation(invocation_state* i_state, F func, int arg_index, ProcessedArgTypes...processed_args, OtherParams&&...other_params) {
		// All conversions from with_block_var to dyn_var will happen INSIDE the lambda and hence the lifetype of the
		// dyn_var will be valid for the run
		// Mark the lambda to be mutable otherwise if func accepts a reference it won't bind correctly 
		// since captured variables are const when captured by value
		auto vtype = std::make_shared<block::scalar_type>();
		vtype->scalar_type_id = block::scalar_type::VOID_TYPE;
		i_state->generated_func_decl->return_type = vtype;
		// Since each argument is wrapped in a tuple, we can expand the tuple and capture 
		// the tuple by value
		i_state->invocation_function = [=]() -> void {func(std::get<0>(processed_args)...);};
	}
};

// 4. When there aren't any more args to fill and return type is not void
template <typename...ProcessedArgTypes, typename ReturnType>
struct extract_signature_impl<std::tuple<ProcessedArgTypes...>, std::tuple<>, ReturnType, void> {
	template <typename F, typename...OtherParams>
	static void fill_invocation(invocation_state* i_state, F func, int arg_index, ProcessedArgTypes...processed_args, OtherParams&&...other_params) {
		// same reason as mutable as above
		i_state->generated_func_decl->return_type = ReturnType::create_block_type();
		// Same reason why tuple are expanded above
		i_state->invocation_function = [=]() -> void {create_return_stmt(func(std::get<0>(processed_args)...));};
	}
};

// f_type_helper extracts argument and return type from lambda or function

// Default case matches for lambdas and redirects to "pointer to member function"
template<typename F>
struct f_type_helper: public f_type_helper<decltype(&F::operator())> {};

// Case for pointer to member function (the actual operator() on lambdas) but the const version
template <typename ClassName, typename ReturnType, typename...ArgTypes>
struct f_type_helper<ReturnType(ClassName::*)(ArgTypes...) const>: f_type_helper<ReturnType(ArgTypes...)> {};

// Same as above except non-cosnt for lambdas declared mutable
template <typename ClassName, typename ReturnType, typename...ArgTypes>
struct f_type_helper<ReturnType(ClassName::*)(ArgTypes...)>: f_type_helper<ReturnType(ArgTypes...)> {};

// Case for regular function and pointer types
template <typename ReturnType, typename...ArgTypes>
struct f_type_helper<ReturnType(*)(ArgTypes...)>: public f_type_helper<ReturnType(ArgTypes...)> {};

template <typename ReturnType, typename...ArgTypes>
struct f_type_helper<ReturnType(ArgTypes...)> {
	using arg_types = std::tuple<ArgTypes...>;
	using ret_type = ReturnType;
};

// Main entry point for extract_signature
template <typename F, typename...OtherArgs>
invocation_state extract_signature(F func, OtherArgs&&... args) {
	using ArgTypes = typename f_type_helper<F>::arg_types;
	using ReturnType = typename f_type_helper<F>::ret_type;
	
	// Create an empty function decl	
	auto func_decl = std::make_shared<block::func_decl>();	
	invocation_state i_state;
	i_state.generated_func_decl = func_decl;
	
	extract_signature_impl<std::tuple<>, ArgTypes, ReturnType>::fill_invocation(&i_state, func, 0, std::forward<OtherArgs>(args)...);

	return i_state;	
}

}
#endif

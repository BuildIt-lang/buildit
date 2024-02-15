#ifndef SIGNATURE_EXTRACT_H
#define SIGNATURE_EXTRACT_H
#include "builder/builder_context.h"
#include "builder/forward_declarations.h"
#include <functional>
#include <type_traits>

namespace builder {

template <typename T, class Enable = void>
struct peel_dyn;

template <typename T>
struct peel_dyn<T, typename std::enable_if<std::is_base_of<var, T>::value>::type> {
	typedef typename T::stored_type type;
};


struct extract_signature_enable;
template <typename T, class Enable = void>
struct filter_var_type {
	constexpr static bool value = false;
};

template <typename T>
struct filter_var_type<T, typename std::enable_if<std::is_base_of<var, T>::value>::type> {
	constexpr static bool value = true;
};

// This template matches a dyn_var argument and pushes a new argument in the
// function declaration
template <typename ClassType, typename RetType, typename T, typename... FutureArgTypes>
struct extract_signature<ClassType, RetType, typename std::enable_if<filter_var_type<T>::value>::type, T,
			 FutureArgTypes...> {
	template <typename... RestArgTypes>
	struct from {
		template <typename... OtherArgs>
		struct with {
			static std::function<void(void)> call(builder_context *context, int arg_count, ClassType func,
							      RestArgTypes &...rest_args, OtherArgs &...other_args) {
				std::string arg_name = "arg" + std::to_string(arg_count);
				T *new_arg = context->assume_variable<T>(arg_name);
				context->current_func_decl->args.push_back(new_arg->block_var);
				return extract_signature<ClassType, RetType, void, FutureArgTypes...>::template from<
				    RestArgTypes..., T &>::template with<OtherArgs &...>::call(context, arg_count + 1,
											       func, rest_args...,
											       *new_arg, other_args...);
			}
		};
	};
};

// This template specialization matches a non dyn_var argument and simply
// forwards values from other_args
template <typename ClassType, typename RetType, typename T, typename... FutureArgTypes>
struct extract_signature<ClassType, RetType, typename std::enable_if<!filter_var_type<T>::value>::type, T,
			 FutureArgTypes...> {
	template <typename... RestArgTypes>
	struct from {
		template <typename TO, typename... OtherArgs>
		struct with {
			static std::function<void(void)> call(builder_context *context, int arg_count, ClassType func,
							      RestArgTypes &...rest_args, TO &to,
							      OtherArgs &...other_args) {
				return extract_signature<ClassType, RetType, void, FutureArgTypes...>::template from<
				    RestArgTypes..., TO &>::template with<OtherArgs &...>::call(context, arg_count + 1,
												func, rest_args..., to,
												other_args...);
			}
		};
	};
};

template <typename ClassType, typename RetType>
struct extract_signature<ClassType, RetType, void> {
	template <typename... RestArgTypes>
	struct from {
		template <typename... OtherArgs>
		struct with {
			// At this point rest of the OtherArgs will just be
			// thrown away
			static std::function<void(void)> call(builder_context *context, int arg_count, ClassType func,
							      RestArgTypes &...rest_args) {
				context->current_func_decl->return_type =
				    type_extractor<typename peel_dyn<RetType>::type>::extract_type();
				return [&, func](void) { create_return_stmt(func(rest_args...)); };
			}
		};
	};
};

template <typename ClassType>
struct extract_signature<ClassType, void, void> {
	template <typename... RestArgTypes>
	struct from {
		template <typename... OtherArgs>
		struct with {
			// At this point rest of the OtherArgs will just be
			// thrown away
			static std::function<void(void)> call(builder_context *context, int arg_count, ClassType func,
							      RestArgTypes &...rest_args) {
				block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
				type->scalar_type_id = block::scalar_type::VOID_TYPE;
				context->current_func_decl->return_type = type;
				return [&, func](void) { func(rest_args...); };
			}
		};
	};
};

template <typename T, typename... OtherArgs>
struct extract_signature_from_lambda;

// Match for lambda
template <typename T, typename... OtherArgs>
struct extract_signature_from_lambda : public extract_signature_from_lambda<decltype(&T::operator())> {};

template <typename ClassType, typename ReturnType, typename... Args, typename... OtherArgs>
struct extract_signature_from_lambda<ReturnType (ClassType::*)(Args...) const, OtherArgs...> {
	static std::function<void(void)> from(builder_context *context, ClassType func, std::string func_name,
					      OtherArgs &...other_args) {
		return extract_signature<ClassType, ReturnType, void,
					 Args...>::template from<>::template with<OtherArgs...>::call(context, 0, func,
												      other_args...);
	}
};

// Match for functions
template <typename ReturnType, typename... Args, typename... OtherArgs>
struct extract_signature_from_lambda<ReturnType (*)(Args...), OtherArgs...> {
	static std::function<void(void)> from(builder_context *context, ReturnType (*func)(Args...),
					      std::string func_name, OtherArgs &...other_args) {
		return extract_signature<ReturnType(Args...), ReturnType, void,
					 Args...>::template from<>::template with<OtherArgs...>::call(context, 0, func,
												      other_args...);
	}
};
} // namespace builder
#endif

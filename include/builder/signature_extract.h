#ifndef SIGNATURE_EXTRACT_H
#define SIGNATURE_EXTRACT_H
#include <functional>
#include "builder/builder_context.h"

namespace builder {
template <typename T>
class type_extractor;

template <typename...AllArgs>
struct extract_signature;

template <typename T>
class dyn_var;


template <typename T>
struct peel_dyn;

template <typename T>
struct peel_dyn<dyn_var<T>> {
	typedef T type;
};

void create_return_stmt(const builder a);

template <typename ClassType, typename RetType, typename T, typename...FutureArgTypes>
struct extract_signature<ClassType, RetType, T, FutureArgTypes...> {
	template <typename...RestArgTypes>
	struct from {
		static std::function<void(void)> call(builder_context *context, int arg_count, ClassType func, RestArgTypes&...rest_args) {
			std::string arg_name = "arg" + std::to_string(arg_count);
			T* new_arg = context->assume_variable<T>(arg_name);
			block::var::Ptr arg = std::make_shared<block::var>();
			arg->var_name = arg_name;
			arg->var_type = type_extractor<typename peel_dyn<T>::type>::extract_type();	
			context->current_func_decl->args.push_back(arg);
			return extract_signature<ClassType, RetType, FutureArgTypes...>::template from<RestArgTypes..., T>::call(context, arg_count + 1, func, rest_args..., *new_arg);
		}
	};
};


template <typename ClassType, typename RetType>
struct extract_signature<ClassType, RetType> {
	template <typename...RestArgTypes>
	struct from {
		static std::function<void(void)> call(builder_context *context, int arg_count, ClassType func, RestArgTypes&...rest_args) {
			context->current_func_decl->return_type = type_extractor<typename peel_dyn<RetType>::type>::extract_type();
			return [&, func] (void) {
				create_return_stmt(func(rest_args...));
			};
		}
	};
};


template <typename ClassType>
struct extract_signature<ClassType, void> {
	template <typename...RestArgTypes>
	struct from {
		static std::function<void(void)> call(builder_context *context, int arg_count, ClassType func, RestArgTypes&...rest_args) {
			block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
			type->scalar_type_id = block::scalar_type::VOID_TYPE;
			context->current_func_decl->return_type = type;
			return [&, func] (void) {
				func(rest_args...);
			};
		}
	};
};


template <typename T>
struct extract_signature_from_lambda;

// Match for lambda
template <typename T>
struct extract_signature_from_lambda: public extract_signature_from_lambda<decltype(&T::operator())> {};

template <typename ClassType, typename ReturnType, typename...Args>
struct extract_signature_from_lambda<ReturnType(ClassType::*)(Args...) const> {	
	static std::function<void(void)> from(builder_context* context, ClassType func, std::string func_name) {
		return extract_signature<ClassType, ReturnType, Args...>::template from<>::call(context, 0, func);
	}
};

// Match for functions
template <typename ReturnType, typename...Args>
struct extract_signature_from_lambda<ReturnType(*)(Args...)> {	
	static std::function<void(void)> from(builder_context* context, ReturnType (*func)(Args...), std::string func_name) {
		return extract_signature<ReturnType(Args...), ReturnType, Args...>::template from<>::call(context, 0, func);
	}
};
}
#endif

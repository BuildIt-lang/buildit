#ifndef SIGNATURE_EXTRACT_H
#define SIGNATURE_EXTRACT_H
#include <functional>
#include "builder/builder_context.h"

namespace builder {

template <typename...AllArgs>
struct extract_signature;


template <typename ClassType, typename RetType, typename T, typename...FutureArgTypes>
struct extract_signature<ClassType, RetType, T, FutureArgTypes...> {
	template <typename...RestArgTypes>
	struct from {
		static std::function<void(void)> call(builder_context *context, int arg_count, ClassType func, RestArgTypes&...rest_args) {
			std::string arg_name = "arg" + std::to_string(arg_count);
			T* new_arg = context->assume_variable<T>(arg_name);
			return extract_signature<ClassType, RetType, FutureArgTypes...>::template from<RestArgTypes..., T>::call(context, arg_count + 1, func, rest_args..., *new_arg);
		}
	};
};


template <typename ClassType, typename RetType>
struct extract_signature<ClassType, RetType> {
	template <typename...RestArgTypes>
	struct from {
		static std::function<void(void)> call(builder_context *context, int arg_count, ClassType func, RestArgTypes&...rest_args) {
			return [&, func] (void) {
				func(rest_args...);
			};
		}
	};
};

template <typename T>
struct extract_signature_from_lambda: public extract_signature_from_lambda<decltype(&T::operator())> {};

template <typename ClassType, typename ReturnType, typename...Args>
struct extract_signature_from_lambda<ReturnType(ClassType::*)(Args...) const> {	
	static std::function<void(void)> from(builder_context* context, ClassType func) {
		return extract_signature<ClassType, ReturnType, Args...>::template from<>::call(context, 0, func);
	}
};
}
#endif

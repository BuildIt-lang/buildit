#ifndef FUNCTION_TYPE_TRAITS
#define FUNCTION_TYPE_TRAITS
#include <tuple>
// Type traits is currently not required. We are able to extract the return type and argument types from a lambda directly for now
template <std::size_t I, class T>
using tuple_element_t = typename std::tuple_element<I, T>::type;

// This implementation of indexSequence is borrowed from @max66's answer from 
// https://stackoverflow.com/questions/49669958/details-of-stdmake-index-sequence-and-stdindex-sequence
// This is required because index_sequence was introduced in C++14. We want to maintain compatibility with C++11 as much as possible. If later we switch to 14, this will be just replaced with std::index_sequence and std::make_index_sequence

// index sequence only
template <std::size_t ...>
struct indexSequence
 { };

template <std::size_t N, std::size_t ... Next>
struct indexSequenceHelper : public indexSequenceHelper<N-1U, N-1U, Next...>
 { };

template <std::size_t ... Next>
struct indexSequenceHelper<0U, Next ... >
 { using type = indexSequence<Next ... >; };

template <std::size_t N>
using makeIndexSequence = typename indexSequenceHelper<N>::type;


// This implementation for function type traits is borrowed from @Nir Friedman's answer at
// https://stackoverflow.com/questions/43560492/how-to-extract-lambdas-return-type-and-variadic-parameters-pack-back-from-gener
// Couldn't figure out the std::index_sequence part myself
// This is required to extract return types and arg types from a lambda reference

namespace builder {
template <typename T>
struct function_traits : public function_traits<decltype(&T::operator())>{
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const> {
	using result_type = ReturnType;
	using arg_tuple = std::tuple<Args...>;
	static constexpr auto arity = sizeof...(Args);
};

template <class T, typename...Types>
struct lambda_function_type_impl;

template <class T, std::size_t...Is>
struct lambda_function_type_impl<T, indexSequence<Is...>> {
	typedef std::function<typename T::result_type(tuple_element_t<Is, typename T::arg_tuple>...)> type;
};

template <typename F>
struct lambda_function_type {
	using traits = function_traits<F>;
	using type = typename lambda_function_type_impl<traits, makeIndexSequence<traits::arity>>::type;
};

}

#endif


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator && (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::and_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator || (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::or_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator + (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::plus_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator - (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::minus_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator * (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::mul_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator / (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::div_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator < (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::lt_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator > (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::gt_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator <= (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::lte_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator >= (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::gte_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator == (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::equals_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator != (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::ne_expr>((ret_type)b);
}


template <typename T1, typename T2>
typename std::enable_if<allowed_builder_type<T1, T2>::value, typename allowed_builder_return<T1, T2>::type>::type operator % (const T1 &a, const T2 &b) {
	typedef typename allowed_builder_return<T1, T2>::type ret_type;
	return ret_type(a).template builder_binary_op<block::mod_expr>((ret_type)b);
}


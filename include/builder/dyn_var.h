#ifndef BUILDER_DYN_VAR_H
#define BUILDER_DYN_VAR_H


namespace builder {

class var {
public:
	// Optional var name
	std::string var_name;
	block::var::Ptr block_var;
	block::decl_stmt::Ptr block_decl_stmt;

	static block::type::Ptr create_block_type(void) {
		// Cannot create block type for abstract class
		assert(false);
	}

	var() = default;

	// operator builder () const;

	explicit operator bool();


	template <typename... types>
	builder operator()(const types &... args) {
		return ((builder) * this)(args...);
	}

	builder operator=(const var &a) { return (builder) * this = a; }
	builder operator[] (const builder &a) {return ((builder) *this)[a];}
	builder operator=(const builder &a) {return (builder)*this = a;}

	virtual ~var() = default;

	builder operator!();
};

template <typename T1, typename T2>
struct allowed_var_type {
	constexpr static bool value = !(std::is_base_of<builder_base<T1>, T1>::value || std::is_base_of<builder_base<T2>, T1>::value) &&
				 (
				      (std::is_base_of<var, T1>::value && std::is_convertible<T2, builder>::value) ||
				      (std::is_base_of<var, T2>::value && std::is_convertible<T1, builder>:: value)
				 );
};

template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator&&(const T1 &a, const T2 &b) { return (builder)a && (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator||(const T1 &a, const T2 &b) { return (builder)a || (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator+(const T1 &a, const T2 &b) { return (builder)a + (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator-(const T1 &a, const T2 &b) { return (builder)a - (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator*(const T1 &a, const T2 &b) { return (builder)a * (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator/(const T1 &a, const T2 &b) { return (builder)a / (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator<(const T1 &a, const T2 &b) { return (builder)a < (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator>(const T1 &a, const T2 &b) { return (builder)a > (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator<=(const T1 &a, const T2 &b) { return (builder)a <= (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator>=(const T1 &a, const T2 &b) { return (builder)a >= (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator==(const T1 &a, const T2 &b) { return (builder)a == (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator!=(const T1 &a, const T2 &b) { return (builder)a != (builder)b;}
template <typename T1, typename T2>
typename std::enable_if<allowed_var_type<T1, T2>::value, builder>::type operator%(const T1 &a, const T2 &b) { return (builder)a % (builder)b;}



template <typename T>
class dyn_var : public var {
public:
	using var::operator=;
	// using var::operator builder;

	builder operator=(const dyn_var<T> &a) { return (builder) * this = a; }
	
	static block::type::Ptr create_block_type(void) { return type_extractor<T>::extract_type(); }
	void create_dyn_var(bool create_without_context = false) {
		if (create_without_context) {
			block::var::Ptr dyn_var = std::make_shared<block::var>();
			dyn_var->var_type = create_block_type();
			block_var = dyn_var;
			return;
		}
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->current_block_stmt != nullptr);
		builder_context::current_builder_context->commit_uncommitted();
		block::var::Ptr dyn_var = std::make_shared<block::var>();
		dyn_var->var_type = create_block_type();
		block_var = dyn_var;
		tracer::tag offset = get_offset_in_function();
		dyn_var->static_offset = offset;
		block_decl_stmt = nullptr;
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block::decl_stmt::Ptr decl_stmt = std::make_shared<block::decl_stmt>();
		decl_stmt->static_offset = offset;
		decl_stmt->decl_var = dyn_var;
		decl_stmt->init_expr = nullptr;
		block_decl_stmt = decl_stmt;
		builder_context::current_builder_context->add_stmt_to_current_block(decl_stmt);
	}
	dyn_var(bool create_without_context = false) { create_dyn_var(create_without_context); }
	dyn_var(const dyn_var<T> &a) : dyn_var<T>((builder)a) {}
	template <typename TO>
	dyn_var(const dyn_var<TO> &a) : dyn_var<TO>((builder)a) {}
	template <typename TO>
	dyn_var(const static_var<TO> &a) : dyn_var<T>((TO)a) {}
	dyn_var(const builder a) {
		builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);
		create_dyn_var();
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block_decl_stmt->init_expr = a.block_expr;
	}
	dyn_var(const int &a) : dyn_var((builder)a) {}
	dyn_var(const double &a) : dyn_var((builder)a) {}
	dyn_var(const float &a) : dyn_var((builder)a) {}
	dyn_var(const std::initializer_list<builder> &_a) {
		std::vector<builder> a(_a);

		assert(builder_context::current_builder_context != nullptr);
		for (unsigned int i = 0; i < a.size(); i++) {
			builder_context::current_builder_context->remove_node_from_sequence(a[i].block_expr);
		}
		create_dyn_var();
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;

		tracer::tag offset = get_offset_in_function();
		block::initializer_list_expr::Ptr list_expr = std::make_shared<block::initializer_list_expr>();
		list_expr->static_offset = offset;
		for (unsigned int i = 0; i < a.size(); i++) {
			list_expr->elems.push_back(a[i].block_expr);
		}
		block_decl_stmt->init_expr = list_expr;
	}
	virtual ~dyn_var() = default;

	template <typename TO>
	builder operator=(const dyn_var<TO> &a) {
		return (builder) * this = a;
	}
	builder operator=(const int &a) { return operator=((builder)a); }
	builder operator=(const double &a) { return operator=((builder)a); }

	template <typename Ts>
	builder operator=(const static_var<Ts> &a) {
		return operator=((builder)a);
	}
};

template<typename BT>
builder_base<BT>::builder_base(const var &a) {
	assert(builder_context::current_builder_context != nullptr);
	block_expr = nullptr;
	if (builder_context::current_builder_context->bool_vector.size() > 0)
		return;
	assert(a.block_var != nullptr);
	tracer::tag offset = get_offset_in_function();

	block::var_expr::Ptr var_expr = std::make_shared<block::var_expr>();
	var_expr->static_offset = offset;

	var_expr->var1 = a.block_var;
	builder_context::current_builder_context->add_node_to_sequence(
	    var_expr);

	block_expr = var_expr;
}
template <typename T>
void create_return_stmt(const dyn_var<T> &a) {
	create_return_stmt((builder)a);	
}

}

#endif

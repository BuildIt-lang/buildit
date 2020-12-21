#ifndef BUILDER_MEMBER_BASE_H
#define BUILDER_MEMBER_BASE_H

#include "builder/forward_declarations.h"

#include "builder/builder_context.h"
#include "blocks/var.h"

namespace builder {
struct member_base {
	virtual block::expr::Ptr get_parent() const;
	virtual ~member_base() = default;
	member_base(member_base*, std::string){}
	member_base(){}
};


template <typename BT>
struct member_base_impl: public member_base {

	typedef BT member_associated_BT;

	member_base* parent;
	std::string member_name;
	member_base_impl() {}
	member_base_impl(member_base *p, std::string s): parent(p), member_name(s) {}
	virtual block::expr::Ptr get_parent() const {
		assert(parent && "Parent cannot be null");
		block::member_access_expr::Ptr member = std::make_shared<block::member_access_expr>();
		member->parent_expr = parent->get_parent();
		builder_context::current_builder_context->remove_node_from_sequence(member->parent_expr);
		member->member_name = member_name;	
		return member;
	}

};

template <typename T>
struct is_pure_member {
	constexpr static bool value = std::is_base_of<member_base, T>::value && !std::is_base_of<builder_base<T>, T>::value && !std::is_base_of<var, T>::value;
};

template <typename T1, typename T2, class Enable=void>
struct allowed_member_return;

template <typename T1, typename T2>
struct allowed_member_return <T1, T2, typename std::enable_if<is_pure_member<T1>::value && is_pure_member<T2>::value>::type> {
	typedef typename T1::member_associated_BT type;
};
template <typename T1, typename T2>
struct allowed_member_return <T1, T2, typename std::enable_if<std::is_convertible<T2, typename T1::member_associated_BT>::value && !std::is_base_of<member_base, T2>::value && !std::is_base_of<builder_base<T2>, T2>::value && !std::is_base_of<var, T2>::value && is_pure_member<T1>::value>::type> {
	typedef typename T1::member_associated_BT type;
};
template <typename T1, typename T2>
struct allowed_member_return <T2, T1, typename std::enable_if<std::is_convertible<T2, typename T1::member_associated_BT>::value && !std::is_base_of<member_base, T2>::value && !std::is_base_of<builder_base<T2>, T2>::value && !std::is_base_of<var, T2>::value && is_pure_member<T1>::value>::type> {
	typedef typename T1::member_associated_BT type;
};

template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator&&(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a && (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator||(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a || (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator+(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a + (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator-(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a - (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator*(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a * (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator/(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a / (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator<(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a < (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator>(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a > (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator<=(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a <= (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator>=(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a >= (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator==(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a == (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator!=(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a != (typename allowed_member_return<T1, T2>::type)b;}
template <typename T1, typename T2>
typename allowed_member_return<T1, T2>::type operator%(const T1 &a, const T2 &b) { return (typename allowed_member_return<T1, T2>::type)a % (typename allowed_member_return<T1, T2>::type)b;}

}

#endif


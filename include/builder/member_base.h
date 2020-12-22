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

}

#endif


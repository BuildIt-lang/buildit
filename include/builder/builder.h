#ifndef BUILDER_H
#define BUILDER_H
#include <memory>
#include <string>
#include "blocks/var.h"
#include "builder/builder_context.h"
#include "builder/static_var.h"

namespace builder {
// Builder objects are always alive only for duration of the RUN/SEQUENCE. 
// Never store pointers to these objects (across runs) or heap allocate them.
class var;
class builder {
public:
	builder() = default;	
	block::expr::Ptr block_expr;
	template <typename T>	
	builder builder_binary_op(const builder &);
	template <typename T>	
	builder builder_unary_op();
	builder operator && (const builder &);	
	builder operator || (const builder &);
	builder operator + (const builder &);
	builder operator - (const builder &);
	builder operator * (const builder &);
	builder operator / (const builder &);
	builder operator < (const builder &);
	builder operator > (const builder &);
	builder operator <= (const builder &);
	builder operator >= (const builder &);
	builder operator == (const builder &);
	builder operator != (const builder &);
	
	builder operator ! ();
	explicit operator bool();

	builder (const int&);

};
builder operator && (const int &a, const builder &);
builder operator || (const int &a, const builder &);
builder operator + (const int &a, const builder &);
builder operator - (const int &a, const builder &);
builder operator * (const int &a, const builder &);
builder operator / (const int &a, const builder &);

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
	
	operator builder() const;

	explicit operator bool();
	builder operator && (const builder &);
	builder operator || (const builder &);
	builder operator + (const builder &);
	builder operator - (const builder &);
	builder operator * (const builder &);
	builder operator / (const builder &);
	builder operator < (const builder &);
	builder operator > (const builder &);
	builder operator <= (const builder &);
	builder operator >= (const builder &);
	builder operator == (const builder &);
	builder operator != (const builder &);

	
	builder operator = (const builder &);

	builder operator ! ();
};


class int_var: public var {
public:
	using var::operator =;
	static block::type::Ptr create_block_type(void) {
		block::scalar_type::Ptr type = std::make_shared<block::scalar_type>();
		type->scalar_type_id = block::scalar_type::INT_TYPE;
		return type;
	}
	void create_int_var(void);
	int_var();
	int_var(const int_var&);
	int_var(const builder&);
	int_var(const int);
};

template <typename base_type> 
class pointer_var: public var {
public:
	using var::operator =;
	static block::type::Ptr create_block_type(void) {
		block::pointer_type::Ptr type = std::make_shared<block::pointer_type>();
		type->pointee_type = base_type::create_block_type();
		return type;
	}
	void create_pointer_var(void);
	pointer_var();
	pointer_var(const pointer_var&);
	pointer_var(const builder&);	
};
template <typename base_type>
void pointer_var<base_type>::create_pointer_var(void) {
	assert(builder_context::current_builder_context != nullptr);
	assert(builder_context::current_builder_context->current_block_stmt != nullptr);
	builder_context::current_builder_context->commit_uncommitted();
	block::var::Ptr pointer_var = std::make_shared<block::var>();	
	pointer_var->var_type = create_block_type();
	block_var = pointer_var;
	tracer::tag offset = get_offset_in_function(builder_context::current_builder_context->current_function);
	block::decl_stmt::Ptr decl_stmt = std::make_shared<block::decl_stmt>();
	decl_stmt->static_offset = offset;
	decl_stmt->decl_var = pointer_var;
	decl_stmt->init_expr = nullptr;
	block_decl_stmt = decl_stmt;
	builder_context::current_builder_context->add_stmt_to_current_block(decl_stmt);
	pointer_var->static_offset = offset;
}	

template <typename base_type>
pointer_var<base_type>::pointer_var() {
	create_pointer_var();
}		
template <typename base_type>
pointer_var<base_type>::pointer_var(const pointer_var<base_type>& a): pointer_var<base_type>((builder)a) {
}
template <typename base_type>
pointer_var<base_type>::pointer_var(const builder& a) {
	builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);
	create_pointer_var();
	block_decl_stmt->init_expr = a.block_expr;	
}

void annotate(std::string);

}



#endif

#ifndef VAR_H
#define VAR_H

#include "blocks/block.h"
#include "util/printer.h"

namespace block {
class type: public block {
public:
	typedef std::shared_ptr<type> Ptr;

	virtual void accept(block_visitor *visitor) {
		visitor->visit(self<type>());	
	}		
	virtual void dump(std::ostream&, int);
	
};
class scalar_type: public type {
public:
	typedef std::shared_ptr<scalar_type> Ptr;
	enum {INT_TYPE} scalar_type_id;
	virtual void accept(block_visitor *visitor) {
		visitor->visit(self<scalar_type>());
	}
	virtual void dump(std::ostream&, int);
};

class pointer_type: public type {
public:
	typedef std::shared_ptr<pointer_type> Ptr;
	type::Ptr pointee_type;
	virtual void accept(block_visitor *visitor) {
		visitor->visit(self<pointer_type>());
	}
	virtual void dump(std::ostream&, int);
};
class var: public block {
public:
	typedef std::shared_ptr<var> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor *visitor) {
		visitor->visit(self<var>());
	}
	// Optional var_name
	std::string var_name;
	
	type::Ptr var_type;
};


}
#endif

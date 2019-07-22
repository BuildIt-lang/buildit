#ifndef VAR_H
#define VAR_H

#include "blocks/block.h"
namespace block {
class var: public block {
public:
	typedef std::shared_ptr<var> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<var>());
	}
	// Optional var_name
	std::string var_name;
};

class int_var: public var {
public:
	typedef std::shared_ptr<int_var> Ptr;
	virtual void dump(std::ostream&, int);
	virtual void accept(block_visitor* visitor) {
		visitor->visit(self<int_var>());
	}
	
};
}
#endif

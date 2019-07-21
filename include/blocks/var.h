#ifndef VAR_H
#define VAR_H

#include "blocks/block.h"
namespace block {
class var: public block {
public:
	typedef util::wrapped_shared_ptr<var> Ptr;
	virtual void dump(std::ostream&, int);
	// Optional var_name
	std::string var_name;
};

class int_var: public var {
public:
	typedef util::wrapped_shared_ptr<int_var> Ptr;
	virtual void dump(std::ostream&, int);
	
};
}
#endif

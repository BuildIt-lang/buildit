#ifndef VAR_H
#define VAR_H

#include "blocks/block.h"
namespace block {
class var: public block {
public:
	typedef std::shared_ptr<var> Ptr;
	// Optional var_name
	std::string var_name;
};

class int_var: public block {
public:
	typedef std::shared_ptr<int_var> Ptr;
	
};
}
#endif

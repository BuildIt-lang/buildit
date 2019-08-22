#include "blocks/var.h"
#include "util/printer.h"

namespace block{
void var::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "VAR (" << var_name << ")" << std::endl;	
}
void type::dump(std::ostream &oss, int indent) {
}
void scalar_type::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "SCALAR_TYPE (";
	if (scalar_type_id == INT_TYPE)
		oss << "INT";
	else if (scalar_type_id == VOID_TYPE)
		oss << "VOID";
	oss << ")" << std::endl;
}
void pointer_type::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "POINTER_TYPE" << std::endl;
	pointee_type->dump(oss, indent+1);
}
void function_type::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "FUNCITON_TYPE" << std::endl;
	return_type->dump(oss, indent+1);
	for (int i = 0; i < arg_types.size(); i++) 
		arg_types[i]->dump(oss, indent+1);
}
}

#include "blocks/var.h"
#include "util/printer.h"

namespace block {
void var::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "VAR (" << var_name << ")" << std::endl;

#ifdef DEBUG_VARS
	printer::indent(oss, indent);
	oss << static_offset.stringify() << std::endl;
#endif
}
void type::dump(std::ostream &oss, int indent) {}
void scalar_type::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "SCALAR_TYPE (";
	if (scalar_type_id == SHORT_INT_TYPE)
		oss << "SHORT_INT";
	else if (scalar_type_id == UNSIGNED_SHORT_INT_TYPE)
		oss << "UNSIGNED_SHORT_INT";
	else if (scalar_type_id == INT_TYPE)
		oss << "INT";
	else if (scalar_type_id == UNSIGNED_INT_TYPE)
		oss << "UNSIGNED_INT";
	else if (scalar_type_id == LONG_INT_TYPE)
		oss << "LONG_INT";
	else if (scalar_type_id == UNSIGNED_LONG_INT_TYPE)
		oss << "UNSIGNED_LONG_INT";
	else if (scalar_type_id == LONG_LONG_INT_TYPE)
		oss << "LONG_LONG_INT";
	else if (scalar_type_id == UNSIGNED_LONG_LONG_INT_TYPE)
		oss << "UNSIGNED_LONG_LONG_INT";
	else if (scalar_type_id == CHAR_TYPE)
		oss << "CHAR";
	else if (scalar_type_id == UNSIGNED_CHAR_TYPE)
		oss << "UNSIGNED_CHAR";
	else if (scalar_type_id == VOID_TYPE)
		oss << "VOID";
	else if (scalar_type_id == FLOAT_TYPE)
		oss << "FLOAT";
	else if (scalar_type_id == DOUBLE_TYPE)
		oss << "DOUBLE";
	oss << ")" << std::endl;
}
void pointer_type::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "POINTER_TYPE" << std::endl;
	pointee_type->dump(oss, indent + 1);
}
void reference_type::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "REFERENCE_TYPE" << std::endl;
	referenced_type->dump(oss, indent + 1);
}
void function_type::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "FUNCITON_TYPE" << std::endl;
	return_type->dump(oss, indent + 1);
	for (unsigned int i = 0; i < arg_types.size(); i++)
		arg_types[i]->dump(oss, indent + 1);
}
void array_type::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "ARRAY_TYPE" << std::endl;
	element_type->dump(oss, indent + 1);
	printer::indent(oss, indent + 1);
	oss << size << std::endl;
}
void builder_var_type::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "BUILDER_VAR_TYPE (";
	if (builder_var_type_id == builder_var_type::DYN_VAR)
		oss << "DYN_VAR";
	else if (builder_var_type_id == builder_var_type::STATIC_VAR)
		oss << "STATIC_VAR";
	oss << ")" << std::endl;
	closure_type->dump(oss, indent + 1);
}
void named_type::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "NAMED_TYPE (";
	oss << type_name << ")" << std::endl;
	for (unsigned int i = 0; i < template_args.size(); i++)
		template_args[i]->dump(oss, indent + 1);
}
} // namespace block

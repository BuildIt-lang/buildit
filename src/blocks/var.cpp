#include "blocks/var.h"
#include "util/printer.h"

namespace block{
void var::dump(std::ostream &oss, int indent) {
}
void int_var::dump(std::ostream &oss, int indent) {
	printer::indent(oss, indent);
	oss << "INT_VAR (" << var_name << ")" << std::endl;

}
}

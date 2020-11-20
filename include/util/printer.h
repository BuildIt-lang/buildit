#ifndef PRINTER_H
#define PRINTER_H
#include <iostream>
namespace printer {
#define INDENT_CHAR ("  ")
static inline void indent(std::ostream &oss, int indent) {
	for (int i = 0; i < indent; i++) {
		oss << INDENT_CHAR;
	}
}
} // namespace printer

#endif

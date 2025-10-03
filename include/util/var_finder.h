#ifndef UTIL_VAR_FINDER_H
#define UTIL_VAR_FINDER_H
#include <string>
#include "util/tracer.h"

namespace utils {
std::string find_variable_name(void *);
std::string find_variable_name_cached(void *, tracer::tag stag);
extern std::string ember_separator;
} // namespace utils

#endif

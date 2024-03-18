#ifndef UTIL_VAR_FINDER_H
#define UTIL_VAR_FINDER_H
#include <string>
namespace util {
std::string find_variable_name(void *);
std::string find_variable_name_cached(void *, std::string tag_string);
extern std::string member_separator;
} // namespace util

#endif

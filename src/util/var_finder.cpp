#include "util/var_finder.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef RECOVER_VAR_NAMES
#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include <dlfcn.h>
#include <memory>
#include <fcntl.h>
#include "elf++.hh"
#include "dwarf++.hh"
#include <map>
#include <algorithm>
#include <iostream>
#include <link.h>

namespace util {

static int backtrace_find_cursor(void* addr, unw_cursor_t *ret, unw_cursor_t *ret_prev) {
	unw_cursor_t cursor, cursor_next, cursor_prev;
	unw_context_t context;
	
	unw_getcontext(&context);
	unw_init_local(&cursor, &context);

	while (1) {
		unw_word_t sp, sp_next;
		unw_get_reg(&cursor, UNW_REG_SP, &sp);
		cursor_next = cursor;
		if (unw_step(&cursor_next) <= 0) {
			return 0;
		}
		unw_get_reg(&cursor_next, UNW_REG_SP, &sp_next);
			
		if ((unsigned long long)sp <= (unsigned long long)addr && (unsigned long long)addr < (unsigned long long) sp_next) {
			*ret = cursor;
			*ret_prev = cursor_prev;
			return 1;
		}
		cursor_prev = cursor;
		cursor = cursor_next;	
	}	
}


static int64_t create_sleb128(const char* &s, const char* e) {
	uint64_t result = 0;
	unsigned shift = 0;
	while(s < e) {
		uint8_t byte = *(uint8_t*)(s++);
		result |= (uint64_t)(byte & 0x7f) << shift;
		shift += 7;
		if ((byte & 0x80) == 0) {
			if (shift < sizeof(result) * 8 && (byte & 0x40))
				result |= -((uint64_t)1 << shift);
			return result;
		}
	}
	return 0;
}

static std::string find_vars_in_f(const dwarf::die &node, long offset, unsigned long long find_offset, char* base_ptr) {
	for (auto &child: node) {
		if (child.tag == dwarf::DW_TAG::variable || child.tag == dwarf::DW_TAG::formal_parameter) {
			if (child.has(dwarf::DW_AT::name) && child.has(dwarf::DW_AT::location)) {
				dwarf::value loc = child.resolve(dwarf::DW_AT::location);
				size_t size;
				const char* ptr = (const char*)loc.as_block(&size);
				const char* end = ptr + size;
				dwarf::DW_OP op = (dwarf::DW_OP)*ptr;
				ptr++;
				if (op == dwarf::DW_OP::fbreg) {
					int64_t off = create_sleb128(ptr, end);
					// Formal parameters sometimes have DW_OP_deref at the end
					if (ptr != end) {
						dwarf::DW_OP op = (dwarf::DW_OP)*ptr;
						if (op == dwarf::DW_OP::deref) {
							unsigned long long value = *((unsigned long long*)(base_ptr + off));
							off = (long long)value - (long long)base_ptr;
						}
					}
					if (off == offset) {
						return child.resolve(dwarf::DW_AT::name).as_string();
					}
				}
			}
		}
	}
	// We have scanned all the variables and haven't found a match
	// Check lexical scopes too
	for (auto &child: node) {
		if (child.tag == dwarf::DW_TAG::lexical_block) {
			if (child.has(dwarf::DW_AT::low_pc) && child.has(dwarf::DW_AT::high_pc)) {
				unsigned long long lp = child.resolve(dwarf::DW_AT::low_pc).as_address();
				unsigned long long hp = child.resolve(dwarf::DW_AT::high_pc).as_uconstant();
				if (find_offset < (lp + hp) && find_offset >= lp) {
					std::string var = find_vars_in_f(child, offset, find_offset, base_ptr);
					if (var != "")
						return var;
				}
			} else if (child.has(dwarf::DW_AT::ranges)) {
				auto ranges = child.resolve(dwarf::DW_AT::ranges).as_rangelist();
				if (ranges.contains(find_offset)) {
					std::string var = find_vars_in_f(child, offset, find_offset, base_ptr);
					if (var != "")
						return var;
				}
			}
		}
	}
	return "";
}

static std::string find_tree(const dwarf::die &node, unsigned long long find_offset, long var_offset, char* base_ptr) {
	if (node.tag == dwarf::DW_TAG::subprogram) {
		// First check if this has base ptr
		if (node.has(dwarf::DW_AT::low_pc) && node.has(dwarf::DW_AT::high_pc)) {
			unsigned long long lp = node.resolve(dwarf::DW_AT::low_pc).as_address();
			unsigned long long hp = node.resolve(dwarf::DW_AT::high_pc).as_uconstant();
			if ((find_offset < (lp + hp) && find_offset >= lp)) {
				std::string var = find_vars_in_f(node, var_offset, find_offset, base_ptr);
				if (var != "")
					return var;
			}
		}
	}
	for (auto &child: node) {
		std::string v = find_tree(child, find_offset, var_offset, base_ptr);
		if (v != "") {
			return v;
		}
	}
	return "";
}

static std::map<std::string, std::shared_ptr<elf::elf>> elf_map;


static std::shared_ptr<elf::elf> find_or_create_elf(const char* fname) {
	std::string path = fname;	
	if (elf_map.find(path) != elf_map.end()) {
		return elf_map[path];
	}
	// Allocate a new dwarf object
	int fd = open(fname, O_RDONLY);
	if (fd < 0) {
		return nullptr;
	}
	std::shared_ptr<elf::elf> ef = std::make_shared<elf::elf>(elf::create_mmap_loader(fd));
	elf_map[path] = ef;
	return ef;
}


static std::string find_variable_with_cursor(void* addr, unw_cursor_t &cursor) {
	unw_word_t ip, base_ptr;
	unw_get_reg(&cursor, UNW_REG_IP, &ip);

	unw_step(&cursor);
	unw_get_reg(&cursor, UNW_REG_SP, &base_ptr);
	long long var_offset = (long long) addr - (long long) (unsigned long long) base_ptr;
	(void) var_offset;

	Dl_info info;
	struct link_map *map;
	if (!dladdr1((void*)ip, &info, (void**)&map, RTLD_DL_LINKMAP)) {
		return "";
	}

	
	unsigned long long find_offset = (unsigned long long)((unsigned long long)ip - (unsigned long long)map->l_addr);
	(void) find_offset;
	// Now that we have obtained the base address and the offset for the function
	// we can load the dwarf
	
	std::shared_ptr<elf::elf> ef = find_or_create_elf(info.dli_fname);
	if (ef == nullptr) 
		return "";
	std::shared_ptr<dwarf::dwarf> dw = std::make_shared<dwarf::dwarf>(dwarf::elf::create_loader(*ef));
	std::string var_name = "";
	for (auto cu: dw->compilation_units()) {
		if (cu.root().has(dwarf::DW_AT::ranges)) {
			auto ranges = cu.root().resolve(dwarf::DW_AT::ranges).as_rangelist();
			if (ranges.contains(find_offset)) {	
				var_name = find_tree(cu.root(), find_offset, var_offset, (char*)base_ptr);
				break;
			}
		}
	}
	return var_name;
}


static std::string find_variable_from_this(void* addr) {
        unw_cursor_t cursor, cursor_prev;
	if (!backtrace_find_cursor(addr, &cursor, &cursor_prev)) {
		return "";
	}
	std::string ret;
	ret = find_variable_with_cursor(addr, cursor);
	if (ret != "")
		return ret;	
	// Sometimes variables are allocated in the caller 
	// frame (for return values and some arguments)
	// Also check the previous frame for the debug info
	ret = find_variable_with_cursor(addr, cursor_prev);
	return ret;
}



std::string find_variable_name(void* addr) {
	return find_variable_from_this(addr);
}


}
#else
namespace util {
std::string find_variable_name(void* addr) {
	return "";
}
}
#endif

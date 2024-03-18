#include "util/var_finder.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <map>

#ifdef RECOVER_VAR_NAMES
#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include <algorithm>
#include <dlfcn.h>
#include <fcntl.h>
#include <iostream>
#include <link.h>
#include <map>
#include <memory>
#include <unistd.h>

#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>

namespace util {

// Default member separator is "_"
// this can be set by callers if they want 
// the separator to be .
std::string member_separator = "_";

static int backtrace_find_cursor(void *addr, unw_cursor_t *ret, unw_cursor_t *ret_prev) {
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

		if ((unsigned long long)sp <= (unsigned long long)addr &&
		    (unsigned long long)addr < (unsigned long long)sp_next) {
			*ret = cursor;
			*ret_prev = cursor_prev;
			return 1;
		}
		cursor_prev = cursor;
		cursor = cursor_next;
	}
}

static std::map<std::string, Dwarf_Debug> debug_map;
static int find_or_create_dbg(const char *fname, Dwarf_Debug *ret) {
	std::string path = fname;
	if (debug_map.find(path) != debug_map.end()) {
		*ret = debug_map[path];
		return 0;
	}
	Dwarf_Debug to_ret;
	Dwarf_Error de;
	int fd = open(fname, O_RDONLY);
	if (fd < 0)
		return -1;
	if (dwarf_init(fd, DW_DLC_READ, NULL, NULL, &to_ret, &de)) {
		close(fd);
		return -1;
	}
	debug_map[path] = to_ret;
	*ret = to_ret;
	return 0;
}
static int dbg_step_cu(Dwarf_Debug dbg) {
	Dwarf_Error de;
	Dwarf_Unsigned hl;
	Dwarf_Half st;
	Dwarf_Off off;
	Dwarf_Half as, ls, xs;
	Dwarf_Sig8 ts;
	Dwarf_Unsigned to, nco;
	Dwarf_Half ct;
	return dwarf_next_cu_header_d(dbg, true, &hl, &st, &off, &as, &ls, &xs, &ts, &to, &nco, &ct, &de);
}

static bool check_die_pc(Dwarf_Debug dbg, Dwarf_Die die, uint64_t addr) {
	Dwarf_Error de;
	Dwarf_Unsigned lopc, hipc;
	Dwarf_Half ret_form;
	enum Dwarf_Form_Class ret_class;

	if (dwarf_lowpc(die, &lopc, &de) == DW_DLV_OK) {
		if (!(dwarf_highpc_b(die, &hipc, &ret_form, &ret_class, &de) == DW_DLV_OK)) {
			hipc = ~0ULL;
		}
		if (ret_class == DW_FORM_CLASS_CONSTANT)
			hipc += lopc;
		if (addr >= lopc && addr < hipc)
			return true;
	}

	// Sometimes DIEs have range lists associated with them
	Dwarf_Attribute attr;

	if (dwarf_attr(die, DW_AT_ranges, &attr, &de) == DW_DLV_OK) {
		Dwarf_Addr base = 0;
		Dwarf_Ranges *ranges;
		Dwarf_Signed cnt;
		Dwarf_Unsigned bytecnt;
		Dwarf_Off off;

		if (dwarf_global_formref(attr, &off, &de) != DW_DLV_OK) {
			return false;
		}

		if (dwarf_get_ranges(dbg, (Dwarf_Off)off, &ranges, &cnt, &bytecnt, &de) != DW_DLV_OK)
			return false;
		for (int i = 0; i < cnt; i++) {
			if (ranges[i].dwr_type == DW_RANGES_END)
				break;
			else if (ranges[i].dwr_type == DW_RANGES_ADDRESS_SELECTION)
				base = ranges[i].dwr_addr2;
			else if (ranges[i].dwr_type == DW_RANGES_ENTRY) {
				lopc = ranges[i].dwr_addr1 + base;
				hipc = ranges[i].dwr_addr2 + base;
				if (addr >= lopc && addr < hipc) {
					dwarf_ranges_dealloc(dbg, ranges, cnt);
					return true;
				}
			}
		}
		dwarf_ranges_dealloc(dbg, ranges, cnt);
	}

	return false;
}

static Dwarf_Die find_cu_die(Dwarf_Debug dbg, uint64_t addr) {
	int ret;
	Dwarf_Error de;
	Dwarf_Die die, ret_die;
	Dwarf_Half tag;
	while ((ret = dbg_step_cu(dbg)) == DW_DLV_OK) {
		die = NULL;
		while (dwarf_siblingof(dbg, die, &ret_die, &de) == DW_DLV_OK) {
			if (die != NULL)
				dwarf_dealloc(dbg, die, DW_DLA_DIE);
			die = ret_die;
			if (dwarf_tag(die, &tag, &de) != DW_DLV_OK)
				continue;
			if (tag == DW_TAG_compile_unit)
				break;
		}
		if (ret_die == NULL) {
			if (die != NULL) {
				dwarf_dealloc(dbg, die, DW_DLA_DIE);
				die = NULL;
			}
			continue;
		}
		if (check_die_pc(dbg, die, addr))
			return die;
	}
	return NULL;
}

static void reset_cu(Dwarf_Debug dbg) {
	int ret;
	while ((ret = dbg_step_cu(dbg)) != DW_DLV_NO_ENTRY) {
		if (ret == DW_DLV_ERROR)
			return;
	}
}

static std::string find_die_name(Dwarf_Debug dbg, Dwarf_Die die) {
	char *name = NULL;
	Dwarf_Error de;
	Dwarf_Attribute at;
	if (dwarf_diename(die, &name, &de) == DW_DLV_OK) {
		return name;
	}
	// There isn't name directly, let's check if there is AT_specification
	if (dwarf_attr(die, DW_AT_specification, &at, &de) == DW_DLV_OK) {
		Dwarf_Off off;
		Dwarf_Die spec;
		dwarf_global_formref(at, &off, &de);
		dwarf_offdie(dbg, off, &spec, &de);
		std::string ret = find_die_name(dbg, spec);
		if (ret != "")
			return ret;
	}
	if (dwarf_attr(die, DW_AT_abstract_origin, &at, &de) == DW_DLV_OK) {
		Dwarf_Off off;
		Dwarf_Die spec;
		dwarf_global_formref(at, &off, &de);
		dwarf_offdie(dbg, off, &spec, &de);
		std::string ret = find_die_name(dbg, spec);
		if (ret != "")
			return ret;
	}
	return "";
}

static void *decode_address_from_die(Dwarf_Debug dbg, Dwarf_Die die, uint64_t frame_base) {
	Dwarf_Attribute at;
	Dwarf_Error de;
	Dwarf_Unsigned no_of_elements = 0;
	Dwarf_Loc_Head_c loclist_head = 0;
	Dwarf_Unsigned op_count;
	Dwarf_Locdesc_c desc;

	Dwarf_Addr expr_low;
	Dwarf_Addr expr_high;

	// Dummy params
	Dwarf_Small d1;
	Dwarf_Small d4;
	Dwarf_Unsigned d5;
	Dwarf_Unsigned d6;

	int lres;
	if (dwarf_attr(die, DW_AT_location, &at, &de) == DW_DLV_OK) {
		lres = dwarf_get_loclist_c(at, &loclist_head, &no_of_elements, &de);
		if (lres != DW_DLV_OK)
			return NULL;
		// std::cout << "For variable, the location has no_of_elems = " << no_of_elements << std::endl;
		lres = dwarf_get_locdesc_entry_c(loclist_head, 0, &d1, &expr_low, &expr_high, &op_count, &desc, &d4,
						 &d5, &d6, &de);

		if (op_count != 1 && op_count != 2)
			return NULL;

		uint64_t res;

		Dwarf_Small op;
		Dwarf_Unsigned opd1 = 0, opd2 = 0, opd3 = 0;
		Dwarf_Unsigned offsetforbranch = 0;
		dwarf_get_location_op_value_c(desc, 0, &op, &opd1, &opd2, &opd3, &offsetforbranch, &de);
		if (op != DW_OP_fbreg)
			return NULL;
		res = frame_base + opd1;

		if (op_count == 2) {
			dwarf_get_location_op_value_c(desc, 1, &op, &opd1, &opd2, &opd3, &offsetforbranch, &de);
			if (op != DW_OP_deref)
				return NULL;
			res = *(uint64_t *)res;
		}
		return (void *)res;

		// std::cout << "For variable, the location has no_of_elems = " << no_of_elements << std::endl;
	}

	return NULL;
}


static int find_var_size(Dwarf_Debug dbg, Dwarf_Die var_die) {
	Dwarf_Attribute attr;
	Dwarf_Error de;
	Dwarf_Off off;	
	Dwarf_Die type_die;
	Dwarf_Unsigned size;

	if (dwarf_attr(var_die, DW_AT_type, &attr, &de) != DW_DLV_OK)
		return -1;

	if (dwarf_global_formref(attr, &off, &de) != DW_DLV_OK)
		return -1;

	if (dwarf_offdie(dbg, off, &type_die, &de) != DW_DLV_OK)
		return -1;

	if (dwarf_attr(type_die, DW_AT_byte_size, &attr, &de) != DW_DLV_OK) 
		return -1;

	if (dwarf_formudata(attr, &size, &de) != DW_DLV_OK) 
		return -1;

	return (int) size;	

}


static std::string find_member_at_offset(Dwarf_Debug dbg, Dwarf_Die var_die, int offset) {
	Dwarf_Attribute attr;
	Dwarf_Error de;
	Dwarf_Off off;	
	Dwarf_Die type_die;
	Dwarf_Half tag;

	if (dwarf_attr(var_die, DW_AT_type, &attr, &de) != DW_DLV_OK)
		return "";

	if (dwarf_global_formref(attr, &off, &de) != DW_DLV_OK)
		return "";

	if (dwarf_offdie(dbg, off, &type_die, &de) != DW_DLV_OK)
		return "";

	// Now we iterate over the members and check their offsets;
	Dwarf_Die child, next_child;

	if (dwarf_child(type_die, &next_child, &de) != DW_DLV_OK) 
		return "";
	child = NULL;

	do {
		if (child != NULL)
			dwarf_dealloc(dbg, child, DW_DLA_DIE);
		child = next_child;
		if (dwarf_tag(child, &tag, &de) != DW_DLV_OK)
			continue;
		if (tag == DW_TAG_member) {
			if (dwarf_attr(child, DW_AT_data_member_location, &attr, &de) != DW_DLV_OK)
				continue;
			Dwarf_Unsigned mem_offset;
			if (dwarf_formudata(attr, &mem_offset, &de) != DW_DLV_OK)
				continue;
			if (offset == (int) mem_offset) {
				char* name = NULL;
				if (dwarf_diename(child, &name, &de) != DW_DLV_OK) {
					dwarf_dealloc(dbg, child, DW_DLA_DIE);
					return "";
				}
				dwarf_dealloc(dbg, child, DW_DLA_DIE);
				return name;	
			}
		}	
	} while (dwarf_siblingof(dbg, child, &next_child, &de) == DW_DLV_OK);

	return "";	
}


static std::string find_var_in_f_tree(Dwarf_Debug dbg, Dwarf_Die in_die, uint64_t addr, uint64_t var_offset,
				      char *base_ptr) {
	Dwarf_Error de;
	Dwarf_Die die, curr_die;
	Dwarf_Half tag;
	// First check all the variables and arguments
	if (dwarf_child(in_die, &curr_die, &de) != DW_DLV_OK) {
		return "";
	}
	die = NULL;
	do {
		if (die != NULL) {
			dwarf_dealloc(dbg, die, DW_DLA_DIE);
		}
		die = curr_die;
		if (dwarf_tag(die, &tag, &de) != DW_DLV_OK)
			continue;
		if (tag == DW_TAG_variable || tag == DW_TAG_formal_parameter) {
			std::string vname = find_die_name(dbg, die);
			if (vname == "")
				continue;
			uint64_t var_addr = (uint64_t)decode_address_from_die(dbg, die, (uint64_t)base_ptr);
			if (var_addr == var_offset) {
				dwarf_dealloc(dbg, die, DW_DLA_DIE);
				return vname;
			}
			// We couldn't find an exact match, now let's look for members
			int var_size = find_var_size(dbg, die);
			if (var_size != -1) {
				if (var_offset >= var_addr && var_offset < var_addr + var_size) {
					int offset = var_offset - var_addr;
					// Add the member name only if we find it
					std::string mem_name = find_member_at_offset(dbg, die, offset);
					if (mem_name != "")
						vname = vname + member_separator + mem_name;
					dwarf_dealloc(dbg, die, DW_DLA_DIE);
					return vname;
				}
			}
		}
	} while (dwarf_siblingof(dbg, die, &curr_die, &de) == DW_DLV_OK);
	// Now check all the lexical scopes
	if (dwarf_child(in_die, &curr_die, &de) != DW_DLV_OK)
		return "";
	die = NULL;
	do {
		if (die != NULL)
			dwarf_dealloc(dbg, die, DW_DLA_DIE);
		die = curr_die;
		if (dwarf_tag(die, &tag, &de) != DW_DLV_OK)
			continue;
		if (tag == DW_TAG_lexical_block) {
			if (check_die_pc(dbg, die, addr)) {
				std::string vname = find_var_in_f_tree(dbg, die, addr, var_offset, base_ptr);
				if (vname != "") {
					dwarf_dealloc(dbg, die, DW_DLA_DIE);
					return vname;
				}
			}
		}
	} while (dwarf_siblingof(dbg, die, &curr_die, &de) == DW_DLV_OK);
	return "";
}

static std::string find_var_in_tree(Dwarf_Debug dbg, Dwarf_Die in_die, uint64_t addr, uint64_t var_offset,
				    char *base_ptr) {
	Dwarf_Error de;
	Dwarf_Die die, curr_die;
	Dwarf_Half tag;

	// Obtain the first child in this die
	if (dwarf_child(in_die, &curr_die, &de) != DW_DLV_OK) {
		return "";
	}
	die = NULL;
	do {
		if (die != NULL) {
			dwarf_dealloc(dbg, die, DW_DLA_DIE);
		}
		die = curr_die;
		if (dwarf_tag(die, &tag, &de) != DW_DLV_OK)
			continue;
		// We are looking for a subprogram (function) that has the current addr
		if (tag == DW_TAG_subprogram) {
			if (check_die_pc(dbg, die, addr)) {
				std::string v = find_var_in_f_tree(dbg, die, addr, var_offset, base_ptr);
				if (v != "") {
					dwarf_dealloc(dbg, die, DW_DLA_DIE);
					return v;
				}
			}
		}
		// This is some other type of node
		std::string v = find_var_in_tree(dbg, die, addr, var_offset, base_ptr);
		if (v != "") {
			dwarf_dealloc(dbg, die, DW_DLA_DIE);
			return v;
		}
	} while (dwarf_siblingof(dbg, die, &curr_die, &de) == DW_DLV_OK);
	return "";
}

static std::string find_variable_with_cursor(void *addr, unw_cursor_t &cursor) {
	unw_word_t ip, base_ptr;
	unw_get_reg(&cursor, UNW_REG_IP, &ip);

	unw_step(&cursor);
	unw_get_reg(&cursor, UNW_REG_SP, &base_ptr);

	Dl_info info;
	struct link_map *map;
	if (!dladdr1((void *)ip, &info, (void **)&map, RTLD_DL_LINKMAP)) {
		return "";
	}

	unsigned long long find_offset = (unsigned long long)((unsigned long long)ip - (unsigned long long)map->l_addr);
	(void)find_offset;

	// Now that we have obtained the base address and the offset for the function
	// we can load the dwarf
	Dwarf_Debug dbg;
	if (find_or_create_dbg(info.dli_fname, &dbg) != 0) {
		return "";
	}

	Dwarf_Die cu_die = find_cu_die(dbg, find_offset);

	std::string vname = find_var_in_tree(dbg, cu_die, find_offset, (uint64_t)addr, (char *)base_ptr);

	dwarf_dealloc(dbg, cu_die, DW_DLV_OK);

	reset_cu(dbg);

	return vname;
}

static std::string find_variable_from_this(void *addr) {
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

std::string find_variable_name(void *addr) {
	return find_variable_from_this(addr);
}

} // namespace util
#else
namespace util {
std::string find_variable_name(void *addr) {
	return "";
}
} // namespace util
#endif

namespace util {
static std::map<std::string, std::string> tag_var_name_map;
std::string find_variable_name_cached(void *addr, std::string tag_string) {
	if (tag_var_name_map.find(tag_string) != tag_var_name_map.end()) {
		return tag_var_name_map[tag_string];
	}

	std::string ret = find_variable_name(addr);
	if (ret != "")
		tag_var_name_map[tag_string] = ret;
	return ret;
}
} // namespace util

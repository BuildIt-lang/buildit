#include "util/source_finder.h"
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <libdwarf/libdwarf.h>
#include <libdwarf/dwarf.h>
#include <sstream>
#include <fstream>
#include <dlfcn.h>
#include <link.h>
#include <iostream>
namespace util {

static std::map<std::string, Dwarf_Debug> debug_map;

static int find_debug_info(const char* filename, Dwarf_Debug* ret) {
	std::string path = filename;
	if (debug_map.find(path) != debug_map.end()) {
		*ret = debug_map[path];
		return 0;
	}
	// Allocate a new debug map
	Dwarf_Debug to_ret;
	Dwarf_Error de;
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		return -1;
	}	
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

static Dwarf_Die find_cu_die(Dwarf_Debug dbg, uint64_t addr) {
	int ret;
	Dwarf_Error de;
	Dwarf_Die die, ret_die;
	Dwarf_Unsigned lopc, hipc;
	Dwarf_Half tag;
	Dwarf_Half ret_form;
	enum Dwarf_Form_Class ret_class;
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
		if (dwarf_lowpc(die, &lopc, &de) == DW_DLV_OK) {
			if (!(dwarf_highpc_b(die, &hipc, &ret_form, &ret_class, &de) == DW_DLV_OK)) {
				hipc = ~0ULL;	
			}
			if (ret_class == DW_FORM_CLASS_CONSTANT)
				hipc += lopc;
			if (addr >= lopc && addr < hipc)
				return die;
		}
	}
	return NULL;
}


static void reset_cu(Dwarf_Debug dbg) {
	int ret;
	while ((ret = dbg_step_cu(dbg)) != DW_DLV_NO_ENTRY) {
		if (ret == DW_DLV_ERROR) {
			return;
		}
	}
}


static std::string find_die_name(Dwarf_Debug dbg, Dwarf_Die die, std::string &linkage_name) {
	char* name = NULL;
	Dwarf_Error de;
	Dwarf_Attribute at;
	if (dwarf_diename(die, &name, &de) == DW_DLV_OK) {		
		if (dwarf_attr(die, DW_AT_linkage_name, &at, &de) == DW_DLV_OK) {
			char* linkage;
			dwarf_formstring(at, &linkage, &de);
			linkage_name = linkage;
		}
		return name;
	}
	// There isn't name directly, let's check if there is AT_specification
	if (dwarf_attr(die, DW_AT_specification, &at, &de) == DW_DLV_OK) {
		Dwarf_Off off;
		Dwarf_Die spec;
		dwarf_global_formref(at, &off, &de);
		dwarf_offdie(dbg, off, &spec, &de);
		std::string ret = find_die_name(dbg, spec, linkage_name);
		if (ret != "")
			return ret;
	} 
	if (dwarf_attr(die, DW_AT_abstract_origin, &at, &de) == DW_DLV_OK) {
		Dwarf_Off off;
		Dwarf_Die spec;
		dwarf_global_formref(at, &off, &de);
		dwarf_offdie(dbg, off, &spec, &de);
		std::string ret = find_die_name(dbg, spec, linkage_name);
		if (ret != "")
			return ret;
	}
	return "";
}

static void find_function_info_with_dbg(Dwarf_Debug dbg, Dwarf_Die die, uint64_t addr, std::string &linkage_ret, std::string &funcname_ret) {
	Dwarf_Half tag;
	Dwarf_Error de;
	Dwarf_Unsigned lopc, hipc;
	Dwarf_Half ret_form;
	enum Dwarf_Form_Class ret_class;

	dwarf_tag(die, &tag, &de);
	if (tag == DW_TAG_subprogram) {
		if (dwarf_lowpc(die, &lopc, &de) == DW_DLV_OK) {
			if (!(dwarf_highpc_b(die, &hipc, &ret_form, &ret_class, &de) == DW_DLV_OK)) {
				hipc = ~0ULL;	
			}
			if (ret_class == DW_FORM_CLASS_CONSTANT)
				hipc += lopc;
			if (addr >= lopc && addr < hipc) {
				std::string name = find_die_name(dbg, die, linkage_ret);
				if (name != "") {
					funcname_ret = name;
				} else {
					funcname_ret = "<unnamed>";
					linkage_ret = "";
				}
			}
			return;
		}			
	} else {
		Dwarf_Die child;
		if (dwarf_child(die, &child, &de) == DW_DLV_OK) {
			while (1) {
				find_function_info_with_dbg(dbg, child, addr, linkage_ret, funcname_ret);
				if (funcname_ret != "")
					return;
				Dwarf_Die sibling;
				if (dwarf_siblingof(dbg, child, &sibling, &de) != DW_DLV_OK)
					break;
				child = sibling;
			}
		}
	}
}

static void find_line_info_with_dbg(Dwarf_Debug dbg, uint64_t addr, int *line_no, const char** fname, std::string &function_name, std::string &linkage_name) {
	*line_no = -1;
	*fname = NULL;
	Dwarf_Die cu_die = find_cu_die(dbg, addr);
	Dwarf_Error de;
	if (cu_die == NULL)
		return;
	Dwarf_Signed lcount;
	Dwarf_Line *lbuf;
	Dwarf_Addr lineaddr, plineaddr = ~0ULL;
	Dwarf_Unsigned lineno, plineno;	
	
	char* filename = NULL;
	char* pfilename = NULL;

	int ret = dwarf_srclines(cu_die, &lbuf, &lcount, &de);
	if (ret != DW_DLV_OK) {
		goto cleanup;
	}	
	int i;
	for (i = 0; i < lcount; i++) {
		if (dwarf_lineaddr(lbuf[i], &lineaddr, &de))
			goto cleanup;
		if (dwarf_lineno(lbuf[i], &lineno, &de))
			goto cleanup;
		if (dwarf_linesrc(lbuf[i], &filename, &de))
			goto cleanup;

		if (addr == lineaddr) {
			*line_no = lineno;	
			*fname = filename;
			break;
		} else if (addr < lineaddr && addr > plineaddr) {
			*line_no = plineno;
			*fname = pfilename;
			break;	
		}
		
		plineaddr = lineaddr;
		plineno = lineno;
		pfilename = filename;
	}
	
	if (*line_no != -1) {
		find_function_info_with_dbg(dbg, cu_die, addr, linkage_name, function_name);
	}
	
cleanup:	
	if (cu_die != NULL)
		dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
	reset_cu(dbg);			
}




int find_line_info(uint64_t addr, int* line_no, const char** filename, std::string &function_name, std::string &linkage_name) {
	*line_no = -1;
	*filename = NULL;

	Dl_info info;
	struct link_map *map = nullptr;
	if (!dladdr1((void*) addr, &info, (void**)&map, RTLD_DL_LINKMAP)) {
		return -1;
	}

	Dwarf_Debug dbg;
	if (find_debug_info(info.dli_fname, &dbg)) {
		return -1;
	}

	uint64_t adjusted_ip = addr - (uint64_t)map->l_addr;
	find_line_info_with_dbg(dbg, adjusted_ip, line_no, filename, function_name, linkage_name);

	if (*line_no != -1)
		return 0;
	return -1;
}

}

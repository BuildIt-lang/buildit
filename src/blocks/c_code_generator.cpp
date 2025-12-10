#include "blocks/c_code_generator.h"
#include <iomanip>
#include <limits>
#include <math.h>
#include <sstream>

namespace block {

#ifdef ENABLE_D2X
void c_code_generator::save_static_info(block::Ptr a) {
	if (!use_d2x)
		return;

	for (auto &keyval : a->static_offset.static_var_key_values) {
		xctx.set_var_here(keyval.first, keyval.second);
	}
	for (auto &addr : a->static_offset.pointers) {
		int line_no = -1;
		const char *filename = NULL;
		std::string function_name, linkage_name;
		if (d2x::util::find_line_info(addr, &line_no, &filename, function_name, linkage_name) == 0) {
			// Skip frames that are in the builder namespace
			// this can be changed to decode the name properly
			if (function_name != "" && linkage_name.rfind("_ZN7builder", 0) != 0) {
				std::string fname = filename;
				xctx.push_source_loc({fname, line_no, function_name, -1});
			}
		}
	}
}
void c_code_generator::nextl(void) {
	oss << std::endl;
	if (!use_d2x)
		return;
	xctx.nextl();
}
#else
void c_code_generator::save_static_info(block::Ptr a) {}
void c_code_generator::nextl(void) {
	oss << std::endl;
}

#endif


struct precedence_t {
	int pred;
	bool is_left_assoc;	
};

static precedence_t get_operator_precedence(expr::Ptr a) {
	// expressions that have no precedence 
	if (isa<var_expr>(a) || isa<const_expr>(a)) 
		return {0, true};

	// Special cases
	if (isa<sq_bkt_expr>(a) && a->getBoolMetadata("deref_is_star")) {
		auto sk = to<sq_bkt_expr>(a);
		if (isa<int_const>(sk->index)) {
			if (to<int_const>(sk->index)->value == 0) {
				return {3, false};
			}	
		}
	}
	if (isa<function_call_expr>(a) || isa<sq_bkt_expr>(a) || isa<member_access_expr>(a)) {
		return {2, true};	
	} else if (isa<unary_minus_expr>(a) || isa<not_expr>(a) || isa<bitwise_not_expr>(a) || isa<addr_of_expr>(a) 
		|| isa<cast_expr>(a)) {
		return {3, false};
	} else if (isa<mul_expr>(a) || isa<div_expr>(a) || isa<mod_expr>(a)) {
		return {5, true};
	} else if (isa<plus_expr>(a) || isa<minus_expr>(a)) {
		return {6, true};
	} else if (isa<lshift_expr>(a) || isa<rshift_expr>(a)) {
		return {7, true};
	} else if (isa<lt_expr>(a) || isa<lte_expr>(a) || isa<gt_expr>(a) || isa<gte_expr>(a)) {
		return {9, true};
	} else if (isa<equals_expr>(a) || isa<ne_expr>(a)) {
		return {10, true};
	} else if (isa<bitwise_and_expr>(a)) {
		return {11, true};
	} else if (isa<bitwise_xor_expr>(a)) {
		return {12, true};
	} else if (isa<bitwise_or_expr>(a)) {
		return {13, true};
	} else if (isa<and_expr>(a)) {
		return {14, true};
	} else if (isa<or_expr>(a)) {
		return {15, true};
	} else if (isa<assign_expr>(a)) {
		return {16, false};
	}
	assert(false && "Invalid operator for precedence");
}

// Determine if bracket should be added around the child
static bool expr_needs_bracket(expr::Ptr parent, expr::Ptr child, bool is_left) {
	precedence_t pp = get_operator_precedence(parent);
	precedence_t pc = get_operator_precedence(child);
	if (pc.pred > pp.pred) return true;
	if (pc.pred < pp.pred) return false;
	// equal case
	if (pc.is_left_assoc == is_left) return false;
	return true;
}

void c_code_generator::handle_child(expr::Ptr parent, expr::Ptr child, bool is_left) {
	bool bracket = expr_needs_bracket(parent, child, is_left);
	if (bracket) oss << "(";
	child->accept(this);
	if (bracket) oss << ")";
}

void c_code_generator::print_struct_type(struct_decl::Ptr a) {
	if (a->is_union) oss << "union ";
	else oss << "struct ";

	oss << a->struct_name;

	if (a->is_decl_only) {
		return;
	}

	if (a->struct_name != "")
		oss << " ";

	oss << "{";
	nextl();

	curr_indent++;
	for (auto mem: a->members) {
		printer::indent(oss, curr_indent);
		mem->accept(this);
		nextl();
	}
	curr_indent--;
	printer::indent(oss, curr_indent);
	oss << "}";
}


void c_code_generator::visit(not_expr::Ptr a) {
	oss << "!";
	handle_child(a, a->expr1, false);
}

void c_code_generator::visit(unary_minus_expr::Ptr a) {
	oss << "-";
	handle_child(a, a->expr1, false);
}

void c_code_generator::visit(bitwise_not_expr::Ptr a) {
	oss << "~";
	handle_child(a, a->expr1, false);
}

void c_code_generator::emit_binary_expr(binary_expr::Ptr a, std::string character) {
	handle_child(a, a->expr1, true);
	oss << " " << character << " ";
	handle_child(a, a->expr2, false);
}
void c_code_generator::visit(and_expr::Ptr a) {
	emit_binary_expr(a, "&&");
}
void c_code_generator::visit(bitwise_and_expr::Ptr a) {
	emit_binary_expr(a, "&");
}
void c_code_generator::visit(or_expr::Ptr a) {
	emit_binary_expr(a, "||");
}
void c_code_generator::visit(bitwise_or_expr::Ptr a) {
	emit_binary_expr(a, "|");
}
void c_code_generator::visit(bitwise_xor_expr::Ptr a) {
	emit_binary_expr(a, "^");
}
void c_code_generator::visit(plus_expr::Ptr a) {
	emit_binary_expr(a, "+");
}
void c_code_generator::visit(minus_expr::Ptr a) {
	emit_binary_expr(a, "-");
}
void c_code_generator::visit(mul_expr::Ptr a) {
	emit_binary_expr(a, "*");
}
void c_code_generator::visit(div_expr::Ptr a) {
	emit_binary_expr(a, "/");
}
void c_code_generator::visit(lt_expr::Ptr a) {
	emit_binary_expr(a, "<");
}
void c_code_generator::visit(gt_expr::Ptr a) {
	emit_binary_expr(a, ">");
}
void c_code_generator::visit(lte_expr::Ptr a) {
	emit_binary_expr(a, "<=");
}
void c_code_generator::visit(gte_expr::Ptr a) {
	emit_binary_expr(a, ">=");
}
void c_code_generator::visit(lshift_expr::Ptr a) {
	emit_binary_expr(a, "<<");
}
void c_code_generator::visit(rshift_expr::Ptr a) {
	emit_binary_expr(a, ">>");
}
void c_code_generator::visit(equals_expr::Ptr a) {
	emit_binary_expr(a, "==");
}
void c_code_generator::visit(ne_expr::Ptr a) {
	emit_binary_expr(a, "!=");
}
void c_code_generator::visit(mod_expr::Ptr a) {
	emit_binary_expr(a, "%");
}
void c_code_generator::visit(var_expr::Ptr a) {
	oss << a->var1->var_name;
	if (a->template_args.size() > 0) {
		oss << "<";
		for (unsigned i = 0; i < a->template_args.size(); i++) {
			a->template_args[i]->accept(this);
			if (i != a->template_args.size() - 1) 
				oss << ", ";
		}
		oss << ">";
	}
}
void c_code_generator::visit(int_const::Ptr a) {
	oss << a->value;
	if (a->is_64bit)
		oss << "ll";
}
void c_code_generator::visit(double_const::Ptr a) {
	oss << std::setprecision(15);
	oss << a->value;
	if (floor(a->value) == a->value)
		oss << ".0";
}
void c_code_generator::visit(float_const::Ptr a) {
	oss << std::setprecision(15);
	oss << a->value;
	if (floor(a->value) == a->value)
		oss << ".0";
	oss << "f";
}
static std::string escapeString(const std::string& input) {
    std::string output;

    for (unsigned char c : input) {
        switch (c) {
            // Standard C Escape Sequences
            case '\a': output += "\\a"; break; // Bell
            case '\b': output += "\\b"; break; // Backspace
            case '\f': output += "\\f"; break; // Form feed
            case '\n': output += "\\n"; break; // Newline
            case '\r': output += "\\r"; break; // Carriage return
            case '\t': output += "\\t"; break; // Horizontal tab
            case '\v': output += "\\v"; break; // Vertical tab
            
            // Critical Syntax Characters
            case '\\': output += "\\\\"; break; // Backslash
            case '"':  output += "\\\""; break; // Double quote
            
            default:
                // If it is a printable character (like 'A', '1', '!', ' '), append it.
                if (std::isprint(c)) {
                    output += c;
                } 
                // Otherwise, escape it as a Hex value (e.g., ESC becomes \x1B).
                else {
                    char buf[5]; // Big enough for \xHH + null terminator
                    std::snprintf(buf, sizeof(buf), "\\x%02X", c);
                    output += buf;
                }
                break;
        }
    }
    return output;
}
void c_code_generator::visit(string_const::Ptr a) {
	oss << "\"" << escapeString(a->value) << "\"";
}
void c_code_generator::visit(assign_expr::Ptr a) {
	handle_child(a, a->var1, true);
	oss << " = ";
	handle_child(a, a->expr1, false);
}

void c_code_generator::visit(expr_stmt::Ptr a) {
	a->expr1->accept(this);
	oss << ";";
	if (!a->annotation.empty()) {
		oss << " //";
		for (auto s: a->annotation)
		oss << s << " ";
	}
}

void c_code_generator::print_pragma(stmt::Ptr s) {
	static std::string pragma_prefix ("pragma: ");
	for (auto a: s->annotation) {
		if (!a.compare(0, pragma_prefix.size(), pragma_prefix)) {
			std::string pragma_value = a.substr(pragma_prefix.size());
			printer::indent(oss, curr_indent);
			oss << "_Pragma(\"" << pragma_value << "\")" << std::endl;
		}
	}
}

void c_code_generator::visit(stmt_block::Ptr a) {
	oss << "{";
	nextl();
	curr_indent += 1;
	for (auto stmt : a->stmts) {
		print_pragma(stmt);
		printer::indent(oss, curr_indent);
		stmt->accept(this);
		save_static_info(stmt);
		nextl();
	}
	curr_indent -= 1;
	printer::indent(oss, curr_indent);

	oss << "}";
}


static std::string addQualifiers(type::Ptr a, bool before = true) {
	std::string ret = "";
	if (before) {
		if (a->is_const) ret += "const ";
		if (a->is_volatile) ret += "volatile ";
	} else {
		if (a->is_const) ret += " const";
		if (a->is_volatile) ret += " volatile";
	}
	return ret;
}
static std::string complex_type_helper(type::Ptr a, std::string name) {
	if (isa<pointer_type>(a)) {
		auto ptr = to<pointer_type>(a);
		std::string decorated = "*" + addQualifiers(ptr, false) + name;
		if (isa<function_type>(ptr->pointee_type) || isa<array_type>(ptr->pointee_type)) {
			decorated = " (" + decorated + ")";
		}
		return complex_type_helper(ptr->pointee_type, decorated);
	} else if (isa<array_type>(a)) {
		auto arr = to<array_type>(a);
		std::string ret = name + "[";
		if (arr->size != -1)
			ret += std::to_string(arr->size);
		ret += "]";
		return complex_type_helper(arr->element_type, ret);
	} else if (isa<function_type>(a)) {
		auto ft = to<function_type>(a);
		std::string ret = name + "(";
		for (unsigned int i = 0; i < ft->arg_types.size(); i++) {
			ret += complex_type_helper(ft->arg_types[i], "");
			if (i + 1 < ft->arg_types.size()) 
				ret += ", ";
		}
		if (ft->is_variadic) {
			if (ft->arg_types.size() > 0) {
				ret += ", ";
			}
			ret += "...";
		}
		ret += ")";
		return complex_type_helper(ft->return_type, ret);
	} else if (isa<scalar_type>(a)) {
		auto st = to<scalar_type>(a);
		std::string tp;
		switch (st->scalar_type_id) {
			case scalar_type::SHORT_INT_TYPE:
				tp = "short int";
				break;
			case scalar_type::UNSIGNED_SHORT_INT_TYPE:
				tp = "unsigned short int";
				break;
			case scalar_type::INT_TYPE:
				tp = "int";
				break;
			case scalar_type::UNSIGNED_INT_TYPE:
				tp = "unsigned int";
				break;
			case scalar_type::LONG_INT_TYPE:
				tp = "long int";
				break;
			case scalar_type::UNSIGNED_LONG_INT_TYPE:
				tp = "unsigned long int";
				break;
			case scalar_type::LONG_LONG_INT_TYPE:
				tp = "long long int";
				break;
			case scalar_type::UNSIGNED_LONG_LONG_INT_TYPE:
				tp = "unsigned long long int";
				break;
			case scalar_type::CHAR_TYPE:
				tp = "char";
				break;
			case scalar_type::UNSIGNED_CHAR_TYPE:
				tp = "unsigned char";
				break;
			case scalar_type::VOID_TYPE:
				tp = "void";
				break;
			case scalar_type::FLOAT_TYPE:
				tp = "float";
				break;
			case scalar_type::DOUBLE_TYPE:
				tp = "double";
				break;
			case scalar_type::LONG_DOUBLE_TYPE:
				tp = "long double";
				break;
			case scalar_type::BOOL_TYPE:
				tp = "bool";
				break;
			default:
				assert(false && "Invalid scalar type");
		}
		return addQualifiers(st) + tp + name;
	} else if (isa<named_type>(a)) {
		auto nt = to<named_type>(a);
		std::string ret = addQualifiers(nt) + nt->type_name;
		if (nt->template_args.size()) {
			ret += "<";
			bool needs_comma = false;
			for (auto a : nt->template_args) {
				if (needs_comma)
					ret += ", ";
				needs_comma = true;
				ret += complex_type_helper(a, "");
			}
			ret += ">";
		}
		return ret + name;
	} else if (isa<anonymous_type>(a)) {
		auto at = to<anonymous_type>(a);
		std::string ret = addQualifiers(a);
		std::stringstream ss;
		c_code_generator gen(ss);
		gen.print_struct_type(to<struct_decl>(at->ref_type));
		return ret + ss.str() +  name;
	} else if (isa<reference_type>(a)) {
		auto ptr = to<reference_type>(a);
		std::string decorated = "&" + addQualifiers(ptr, false) + name;
		if (isa<function_type>(ptr->referenced_type) || isa<array_type>(ptr->referenced_type)) {
			decorated = " (" + decorated + ")";
		}
		return complex_type_helper(ptr->referenced_type, decorated);
	} else if (isa<builder_var_type>(a)) {
		auto bt = to<builder_var_type>(a);
		std::string ret = addQualifiers(bt);
		if (bt->builder_var_type_id == builder_var_type::DYN_VAR)
			ret += "builder::dyn_var<";
		else if (bt->builder_var_type_id == builder_var_type::STATIC_VAR)
			ret += "builder::static_var<";
		ret += complex_type_helper(bt->closure_type, "");
		ret += ">";
		return ret + name;
	}
	a->dump(std::cerr, 0);
	assert(false && "Invalid type");
}

void c_code_generator::visit(scalar_type::Ptr type) {
	oss << complex_type_helper(type, "");
}
void c_code_generator::visit(named_type::Ptr type) {
	oss << complex_type_helper(type, "");
}
void c_code_generator::visit(anonymous_type::Ptr type) {
	oss << complex_type_helper(type, "");
}
void c_code_generator::visit(pointer_type::Ptr type) {
	oss << complex_type_helper(type, "");
}
void c_code_generator::visit(reference_type::Ptr type) {
	oss << complex_type_helper(type, "");
}
void c_code_generator::visit(array_type::Ptr type) {
	oss << complex_type_helper(type, "");
}
void c_code_generator::visit(builder_var_type::Ptr type) {
	oss << complex_type_helper(type, "");
}
void c_code_generator::visit(var::Ptr var) {
	oss << var->var_name;
}

void c_code_generator::visit(decl_stmt::Ptr a) {
	if (a->is_typedef) 
		oss << "typedef ";
	if (a->is_extern)
		oss << "extern ";
	if (a->is_static)
		oss << "static ";
	oss << complex_type_helper(a->decl_var->var_type, " " + a->decl_var->var_name);
	if (a->decl_var->hasMetadata<std::vector<std::string>>("attributes")) {
		const auto &attributes = a->decl_var->getMetadata<std::vector<std::string>>("attributes");
		for (auto attr : attributes) {
			oss << " " << attr;
		}
	}
	if (a->init_expr == nullptr) {
		oss << ";";
	} else {
		oss << " = ";
		a->init_expr->accept(this);
		oss << ";";
	}
}
void c_code_generator::visit(if_stmt::Ptr a) {
	oss << "if (";
	a->cond->accept(this);
	oss << ")";
	if (isa<stmt_block>(a->then_stmt)) {
		oss << " ";
		a->then_stmt->accept(this);
		oss << " ";
	} else {
		save_static_info(a);
		nextl();
		curr_indent++;
		printer::indent(oss, curr_indent);
		a->then_stmt->accept(this);
		save_static_info(a);
		nextl();
		curr_indent--;
	}

	if (isa<stmt_block>(a->else_stmt)) {
		if (to<stmt_block>(a->else_stmt)->stmts.size() == 0)
			return;
		oss << "else";
		oss << " ";
		a->else_stmt->accept(this);
	} else {
		oss << "else";
		save_static_info(a);
		nextl();
		curr_indent++;
		printer::indent(oss, curr_indent);
		a->else_stmt->accept(this);
		curr_indent--;
	}
}
void c_code_generator::visit(case_stmt::Ptr a) {
	if (a->is_default) {
		oss << "default";
	} else {
		oss << "case ";
		a->case_value->accept(this);
	}
	oss << ": ";
	save_static_info(a);
	if (a->branch)
		a->branch->accept(this);
}
void c_code_generator::visit(switch_stmt::Ptr a) {
	oss << "switch(";
	a->cond->accept(this);
	oss << ") {";
	save_static_info(a);
	nextl();
	curr_indent++;
	for (auto c: a->cases) {
		printer::indent(oss, curr_indent);
		c->accept(this);
		nextl();
	}
	curr_indent--;
	printer::indent(oss, curr_indent);
	oss << "}";
}
void c_code_generator::visit(while_stmt::Ptr a) {
	oss << "while (";
	a->cond->accept(this);
	oss << ")";
	if (isa<stmt_block>(a->body)) {
		save_static_info(a);
		oss << " ";
		a->body->accept(this);
	} else {
		save_static_info(a);
		nextl();
		curr_indent++;
		printer::indent(oss, curr_indent);
		a->body->accept(this);
		curr_indent--;
	}
}
void c_code_generator::visit(for_stmt::Ptr a) {
	oss << "for (";
	a->decl_stmt->accept(this);
	oss << " ";
	a->cond->accept(this);
	oss << "; ";
	a->update->accept(this);
	oss << ")";
	if (isa<stmt_block>(a->body)) {
		save_static_info(a);
		oss << " ";
		a->body->accept(this);
	} else {
		save_static_info(a);
		nextl();
		curr_indent++;
		printer::indent(oss, curr_indent);
		a->body->accept(this);
		curr_indent--;
	}
}
void c_code_generator::visit(break_stmt::Ptr a) {
	oss << "break;";
}
void c_code_generator::visit(continue_stmt::Ptr a) {
	oss << "continue;";
}
void c_code_generator::visit(sq_bkt_expr::Ptr a) {
	bool star_added = false;
	if (a->getBoolMetadata("deref_is_star")) {
		if (isa<int_const>(a->index)) {
			auto index = to<int_const>(a->index);
			if (index->value == 0) {
				star_added = true;
				oss << "*";
			}
		}
	}

	handle_child(a, a->var_expr, true);

	if (!star_added) {
		oss << "[";
		// index never needs to be bracketed no matter what
		a->index->accept(this);
		oss << "]";
	}
}
void c_code_generator::visit(function_call_expr::Ptr a) {
	handle_child(a, a->expr1, true);
	oss << "(";
	for (unsigned int i = 0; i < a->args.size(); i++) {
		// We don't have a comma operator, so we dont need brackets
		a->args[i]->accept(this);
		if (i != a->args.size() - 1)
			oss << ", ";
	}
	oss << ")";
}
void c_code_generator::visit(initializer_list_expr::Ptr a) {
	oss << "{";
	for (unsigned int i = 0; i < a->elems.size(); i++) {
		// We don't have a comma operator, so we dont need brackets
		a->elems[i]->accept(this);
		if (i != a->elems.size() - 1)
			oss << ", ";
	}
	oss << "}";
}
void c_code_generator::visit(func_decl::Ptr a) {

#ifdef ENABLE_D2X
	if (use_d2x) {
		oss << xctx.begin_section();
	}
#endif

	printer::indent(oss, curr_indent);
	if (a->is_static)
		oss << "static ";
	if (a->is_inline)
		oss << "inline ";
	
	a->return_type->accept(this);
	if (a->hasMetadata<std::vector<std::string>>("attributes")) {
		const auto &attributes = a->getMetadata<std::vector<std::string>>("attributes");
		for (auto attr : attributes) {
			oss << " " << attr;
		}
	}
	oss << " " << a->func_name;
	oss << " (";
	bool printDelim = false;
	for (auto arg : a->args) {
		if (printDelim)
			oss << ", ";
		printDelim = true;
		oss << complex_type_helper(arg->var_type, " " + arg->var_name);
	}
	if (a->is_variadic) {
		oss << ", ...";
		printDelim = true;
	}
	if (!printDelim)
		oss << "void";
	oss << ")";
	if (decl_only || a->is_decl_only) {
		oss << ";";
		return;
	}
	if (isa<stmt_block>(a->body)) {
		oss << " ";
		a->body->accept(this);
		save_static_info(a);
		nextl();
	} else {
		oss << std::endl;
		curr_indent++;
		printer::indent(oss, curr_indent);
		a->body->accept(this);
		save_static_info(a);
		nextl();
		curr_indent--;
	}
#ifdef ENABLE_D2X
	if (use_d2x) {
		xctx.emit_function_info(oss);
		xctx.end_section();
	}
#endif
}
void c_code_generator::visit(struct_decl::Ptr a) {
	printer::indent(oss, curr_indent);
	oss << "struct " << a->struct_name << " {";
	nextl();
	curr_indent++;
	for (auto mem: a->members) {
		printer::indent(oss, curr_indent);
		mem->accept(this);
		nextl();
	}
	curr_indent--;
	printer::indent(oss, curr_indent);
	oss << "};";
}
void c_code_generator::visit(goto_stmt::Ptr a) {
	// a->dump(oss, 1);
	oss << "goto ";
	oss << a->label1->label_name << ";";
}
void c_code_generator::visit(label_stmt::Ptr a) {
	oss << a->label1->label_name << ":";
}
void c_code_generator::visit(return_stmt::Ptr a) {
	oss << "return ";
	a->return_val->accept(this);
	oss << ";";
}
void c_code_generator::visit(member_access_expr::Ptr a) {
	if (isa<sq_bkt_expr>(a->parent_expr)) {
		sq_bkt_expr::Ptr parent = to<sq_bkt_expr>(a->parent_expr);
		if (parent->getBoolMetadata("deref_is_star")) {
			if (isa<int_const>(parent->index)) {
				auto index = to<int_const>(parent->index);
				if (index->value == 0) {
					handle_child(a, parent->var_expr, true);
					oss << "->" << a->member_name;
					return;
				}
			}
		}
	}


	handle_child(a, a->parent_expr, true);
	oss << "." << a->member_name;
}
void c_code_generator::visit(addr_of_expr::Ptr a) {
	oss << "&";
	handle_child(a, a->expr1, false);
}
void c_code_generator::visit(cast_expr::Ptr a) {
	oss << "(";
	a->type1->accept(this);
	oss << ")";
	handle_child(a, a->expr1, false);
}

} // namespace block

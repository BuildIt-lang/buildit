#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <iostream>

using builder::dyn_var;
using builder::static_var;

const char *bf_program;
dyn_var<void(int)> *print_value_ptr;
dyn_var<int()> *get_value_ptr;

static int find_matching_closing(int pc) {
	int count = 1;
	while (bf_program[pc] != 0 && count > 0) {
		pc++;
		if (bf_program[pc] == '[')
			count++;
		else if (bf_program[pc] == ']')
			count--;
	}
	return pc;
}
static int find_matching_opening(int pc) {
	int count = 1;
	while (pc >= 0 && count > 0) {
		pc--;
		if (bf_program[pc] == '[')
			count--;
		else if (bf_program[pc] == ']')
			count++;
	}
	return pc;
}
// BF interpreter
static void interpret_bf(void) {
	auto &get_value = *get_value_ptr;
	auto &print_value = *print_value_ptr;

	dyn_var<int> pointer = 0;
	static_var<int> pc = 0;
	dyn_var<int[256]> tape = {0};
	while (bf_program[pc] != 0) {
		if (bf_program[pc] == '>') {
			pointer = pointer + 1;
		} else if (bf_program[pc] == '<') {
			pointer = pointer - 1;
		} else if (bf_program[pc] == '+') {
			tape[pointer] = (tape[pointer] + 1) % 256;
		} else if (bf_program[pc] == '-') {
			tape[pointer] = (tape[pointer] - 1) % 256;
		} else if (bf_program[pc] == '.') {
			print_value(tape[pointer]);
		} else if (bf_program[pc] == ',') {
			tape[pointer] = get_value();
		} else if (bf_program[pc] == '[') {
			int closing = find_matching_closing(pc);
			if (tape[pointer] == 0) {
				pc = closing;
			}
		} else if (bf_program[pc] == ']') {
			int opening = find_matching_opening(pc);
			pc = opening - 1;
		}
		pc += 1;
	}
}
static void print_wrapper_code(std::ostream &oss) {
	oss << "#include <stdio.h>\n";
	oss << "#include <stdlib.h>\n";
	oss << "void print_value(int x) {printf(\"%c\", x);}\n";
	oss << "int get_value(void) {char x; scanf(\" %c\", &x); return x;}\n";
	oss << "int main(int argc, char* argv[]) ";
}
int main(int argc, char *argv[]) {
	builder::builder_context context;

	// BF program that prints hello world
	bf_program = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.++++++"
		     "+..+++.>>.<-.<.+++.------.--------.>>+.>++.";

	print_value_ptr = context.assume_variable<dyn_var<void(int)>>("print_value");
	get_value_ptr = context.assume_variable<dyn_var<int(void)>>("get_value");

	auto ast = context.extract_ast_from_function(interpret_bf);

	print_wrapper_code(std::cout);
	block::c_code_generator::generate_code(ast, std::cout, 0);
	return 0;
}

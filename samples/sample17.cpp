#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/c_code_generator.h"

using int_var = builder::int_var;
template <typename r_type, typename... a_types>
using function_var = builder::function_var<r_type, a_types...>;
using void_var = builder::void_var;
template <typename T>
using pointer_var = builder::pointer_var<T>;
using builder::static_var;


const char *bf_program;
function_var<void_var, int_var> *print_value;
function_var<int_var> *get_value;
function_var<pointer_var<void_var>> *malloc_func;
function_var<void_var, pointer_var<void_var>> *free_func;

int find_matching_closing(int pc) {
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
int find_matching_opening(int pc) {
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
void interpret_bf(void) {
	int_var pointer = 0;
	static_var<int> pc = 0;
	pointer_var<int_var> tape;
	tape = (*malloc_func)(256);
	while (bf_program[pc] != 0) {
		if (bf_program[pc] == '>') {
			pointer = pointer + 1;
		} else if (bf_program[pc] == '<') {
			pointer = pointer - 1;
		} else if (bf_program[pc] == '+') {
			tape[pointer] = tape[pointer] + 1;
		} else if (bf_program[pc] == '-') {
			tape[pointer] = tape[pointer] - 1;
		} else if (bf_program[pc] == '.') {
			(*print_value) (tape[pointer]);
		} else if (bf_program[pc] == ',') {
			tape[pointer] = (*get_value)();
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
	(*free_func)(tape);
	
}
void print_wrapper_code(std::ostream& oss) {
	oss << "#include <stdio.h>\n";
	oss << "#include <stdlib.h>\n";
	oss << "void print_value(int x) {printf(\"%c\", x);}\n";
	oss << "int get_value(void) {char x; scanf(\" %c\", &x); return x;}\n";
	oss << "int main(int argc, char* argv[]) ";
}
int main(int argc, char* argv[]) {
	builder::builder_context context;
	
	// BF program that prints hello world
	bf_program = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";

	print_value = context.assume_variable<function_var<void_var, int_var>>("print_value");
	get_value = context.assume_variable<function_var<int_var>>("get_value");
	malloc_func = context.assume_variable<function_var<pointer_var<void_var>>>("malloc");
	free_func = context.assume_variable<function_var<void_var, pointer_var<void_var>>>("free");

	auto ast = context.extract_ast_from_function(interpret_bf);	
	
	print_wrapper_code(std::cout);		
	block::c_code_generator::generate_code(ast, std::cout, 0);	
	return 0;
}

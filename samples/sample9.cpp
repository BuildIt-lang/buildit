#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
#include "blocks/annotation_finder.h"
using int_var = builder::int_var;



// A simple for loop with break and continue
void foo(void) {
	builder::annotate("s1");
	int_var a = 0;
	builder::annotate("s2");
	for (int_var b = 0; b < 10; b = b + 1) {
		if (b == 5)
			continue;
		a = a + b;
		builder::annotate("s3");
		if (a > 25)
			break;
	}
}

int main(int argc, char* argv[]) {
	builder::builder_context context;
	auto ast = context.extract_ast_from_function(foo);

	block::stmt::Ptr s1_stmt = block::annotation_finder::find_annotation(ast, "s1");
	if (s1_stmt != nullptr) {
		s1_stmt->dump(std::cout, 0);
	}

	block::stmt::Ptr s2_stmt = block::annotation_finder::find_annotation(ast, "s2");
	if (s2_stmt != nullptr) {
		s2_stmt->dump(std::cout, 0);
	}
	block::stmt::Ptr s3_stmt = block::annotation_finder::find_annotation(ast, "s3");
	if (s3_stmt != nullptr) {
		s3_stmt->dump(std::cout, 0);
	}
	return 0;
}

#include "builder/builder_context.h"
#include "builder/builder.h"
#include <iostream>
int main(int argc, char* argv[]) {
	builder::builder_context *context = new builder::builder_context();
	
	builder::int_var a(context);
	builder::int_var b(context);

	a && b;

	context->extract_ast()->dump(std::cout, 0);

		
	return 0;
	
}

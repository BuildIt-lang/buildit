/*NO_TEST*/
#include "blocks/matchers/patterns.h"

using namespace block::pattern;

int main(int argc, char* argv[]) {
	auto p = plus_expr(var("a"), var("a"));
	return 0;
}

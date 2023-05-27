#include "blocks/c_code_generator.h"
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include <cstring>
#include <iostream>

template <typename T>
using dyn = builder::dyn_var<T>;
using builder::static_var;

static dyn<int(char *)> d_strlen(builder::as_global("strlen"));

static bool is_normal(char m) {
	return (m >= 'a' && m <= 'z') || (m >= 'A' && m <= 'Z') || (m >= '0' && m <= '9');
}
static void progress(const char *re, static_var<char> *next, int p) {
	int ns = p + 1;
	if ((int)strlen(re) == ns) {
		next[ns] = true;
	} else if (is_normal(re[ns]) || '.' == re[ns]) {
		next[ns] = true;
		if ('*' == re[ns + 1]) {
			// We are allowed to skip this
			// so just progress again
			progress(re, next, ns + 1);
		}
	} else if ('*' == re[ns]) {
		next[p] = true;
		progress(re, next, ns);
	}
}

static dyn<int> match_regex(const char *re, dyn<char *> str) {
	// allocate two state vectors
	const int re_len = strlen(re);
	static_var<char> *current = new static_var<char>[re_len + 1];
	static_var<char> *next = new static_var<char>[re_len + 1];
	for (static_var<int> i = 0; i < re_len + 1; i++)
		current[i] = next[i] = 0;
	progress(re, current, -1);
	dyn<int> str_len = d_strlen(str);
	dyn<int> to_match = 0;
	while (to_match < str_len) {
		// Don’t do anything for $.
		static_var<int> early_break = -1;
		for (static_var<int> state = 0; state < re_len; ++state)
			if (current[state]) {
				static_var<char> m = re[state];
				if (is_normal(m)) {
					if (-1 == early_break) {
						// Normal character
						if (str[to_match] == m) {
							progress(re, next, state);
							// If a match happens, it
							// cannot match anything else
							// Setting early break
							// avoids unnecessary checks
							early_break = m;
						}
					} else if (early_break == m) {
						// The comparison has been done
						// already, let us not repeat
						progress(re, next, state);
					}
				} else if ('.' == m) {
					progress(re, next, state);
				} else {
					printf("Invalid Character(%c)\n", (char)m);
					return false;
				}
			}

		// All the states have been checked
		// Now swap the states and clear next
		static_var<int> count = 0;
		for (static_var<int> i = 0; i < re_len + 1; i++) {
			current[i] = next[i];
			next[i] = false;
			if (current[i])
				count++;
		}
		if (count == 0)
			return false;
		to_match = to_match + 1;
	}
	// Now that the string is done,
	// we should have $ in the state
	static_var<int> is_match = (char)current[re_len];
	for (static_var<int> i = 0; i < re_len + 1; i++) {
		next[i] = 0;
		current[i] = 0;
	}
	return is_match;
}

int main(int argc, char *argv[]) {
	builder::builder_context context;
	context.feature_unstructured = true;
	context.run_rce = true;
	auto ast = context.extract_function_ast(match_regex, "match_re", "ab*c*d");
	std::cout << "#include <string.h>" << std::endl;
	block::c_code_generator::generate_code(ast, std::cout);
	return 0;
}

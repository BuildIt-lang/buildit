#ifndef BUILDER_CONTEXT
#define BUILDER_CONTEXT
#include "blocks/basic_blocks.h"
#include "blocks/expr.h"
#include "blocks/stmt.h"
#include "builder/forward_declarations.h"
#include "builder/signature_extract.h"
#include "builder/run_states.h"
#include <functional>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdint>

namespace builder {

void lambda_wrapper(std::function<void(void)>);
void lambda_wrapper_close(void);

class builder_context {
public:
	// Flags are just constants that shouldn't get updated anyway
	// Flags for controlling BuildIt extraction
	// and code generation behavior
	bool run_rce = false;
	bool feature_unstructured = false;
	bool dynamic_use_cxx = false;
	std::string dynamic_compiler_flags = "";
	std::string dynamic_header_includes = "";
	bool enable_d2x = false;	

	void extract_function_ast_impl(invocation_state*);
	block::stmt::Ptr extract_ast_from_run(run_state*);

	// Old API still used by some samples. TODO: phase out
	
	block::stmt::Ptr extract_ast_from_function(std::function<void(void)> func) {
		invocation_state i_state;
		i_state.generated_func_decl = std::make_shared<block::func_decl>();
		i_state.b_ctx = this;
		i_state.invocation_function = func;	
		extract_function_ast_impl(&i_state);
		return i_state.generated_func_decl->body;
	}

	template <typename F, typename... OtherArgs>
	block::stmt::Ptr extract_function_ast(F func_input, std::string func_name, OtherArgs &&...other_args) {
		// Establish a invocation state
		invocation_state i_state = extract_signature<F>(func_input, std::forward<OtherArgs>(other_args)...);
		i_state.b_ctx = this;
		// Set the name of the function 
		i_state.generated_func_decl->func_name = func_name;
		extract_function_ast_impl(&i_state);
		return i_state.generated_func_decl;
	}

};

} // namespace builder


#endif

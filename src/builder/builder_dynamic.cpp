#include "builder/builder_dynamic.h"

namespace builder {
void* compile_asts(builder_context context, std::vector<block::block::Ptr> asts, std::string lookup_name) {
	std::string source_name = get_temp_filename(context);		
	std::ofstream oss(source_name);	
	oss << context.dynamic_header_includes << std::endl;

	if (context.dynamic_use_cxx) 
		oss << "extern \"C\" { " << std::endl;

	// First spit out all the declarations
	for (auto x: asts) {
		block::c_code_generator::generate_code(x, oss, 0, true);
	}
	// Then the actual implementations
	for (auto x: asts) {
		block::c_code_generator::generate_code(x, oss, 0);
	}


	if (context.dynamic_use_cxx) 
		oss << "}" << std::endl; // For the extern "C"
	oss.close();

	return compile_and_return_ptr(context, source_name, lookup_name);
}



}

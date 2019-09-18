
namespace builder {
void lambda_wrapper_impl(void);
void lambda_wrapper(void);
void function_wrapper_impl(void);
void function_wrapper(void);
int tail_call_guard;

void lambda_wrapper(void) {
	lambda_wrapper_impl();
	tail_call_guard += 1;
}
void function_wrapper(void) {
	function_wrapper_impl();
	tail_call_guard += 1;
}
}

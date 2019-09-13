
namespace builder {
void lambda_wrapper_impl(void);
void lambda_wrapper(void);
int tail_call_guard;

void lambda_wrapper(void) {
	lambda_wrapper_impl();
	tail_call_guard += 1;
}
}

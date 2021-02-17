
namespace builder {
void lambda_wrapper_impl(void);
void lambda_wrapper(void);
void lambda_wrapper_close(void);

int tail_call_guard;

void lambda_wrapper(void) {
	lambda_wrapper_impl();
	tail_call_guard += 1;
}
void lambda_wrapper_close(void) {
}
} // namespace builder

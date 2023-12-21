#include <functional>
namespace builder {
void lambda_wrapper(std::function<void(void)>);
void lambda_wrapper_close(void);
void coroutine_wrapper(std::function<void(void)>);
void coroutine_wrapper_close(void);

int tail_call_guard;

void lambda_wrapper(std::function<void(void)> f) {
	f();
	tail_call_guard += 1;
}
void lambda_wrapper_close(void) {}

void coroutine_wrapper(std::function<void(void)> f) {
	f();
	tail_call_guard += 1;
}
void coroutine_wrapper_close(void) {}
} // namespace builder

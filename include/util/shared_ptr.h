#ifndef SHARED_PTR
#define SHARED_PTR

#include <memory>

namespace util {
template <typename T>
class wrapped_shared_ptr: public std::shared_ptr<T>{
public:
	wrapped_shared_ptr(): std::shared_ptr<T>() {
	}
	wrapped_shared_ptr(const std::shared_ptr<T> &other): std::shared_ptr<T>(other) {
	}	
	wrapped_shared_ptr(const nullptr_t &n): std::shared_ptr<T>(n) {
	}
	template<typename O>
	wrapped_shared_ptr(const wrapped_shared_ptr<O> &other): std::shared_ptr<T>(*((std::shared_ptr<O>*)(&other))) {
	}
	
	operator bool() {
		return this != nullptr;
	}
	
};

}
#endif

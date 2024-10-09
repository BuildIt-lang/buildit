#ifndef BUILDER_ARRAY_H
#define BUILDER_ARRAY_H
#include "builder/dyn_var.h"
#include "builder/static_var.h"

namespace builder {


// If the array is of dyn_vars, we initialize with a 
// builder, anything else and we directly initialize with T
// This avoids unnecessary copies unless entirely necessary
template <typename T>
struct initializer_selector {
	typedef const T type;
};

template <typename T>
struct initializer_selector<dyn_var<T>> {
	typedef builder type;
};



template <typename T, size_t size = 0>
class array {
private:
	T *m_arr = nullptr;
	size_t actual_size = 0;

public:
	array() {
		if (size) {
			actual_size = size;
			m_arr = (T *)new char[sizeof(T) * actual_size];
			for (static_var<size_t> i = 0; i < actual_size; i++) {
				new (m_arr + i) T();
			}

			// static tags for array nodes need to be adjusted
			// so they are treated different from each other despite
			// being declared at the same location.
			// dyn_arr are special case of vars that escape their static scope but still
			// shouldn't be treated together
			// We do this by adding additional metadata on all of them
			
			// We are removing the metadata for allow_escape_scope because it is not being 
			// used anyway right now. Enabling this would require us to set it as a variable
			// inside the context so all the dynamic variables constructed in this block would have
			// the metadata set

		}
	}
	// We need a SFINAE constructor of anything that is convertible to T
	// but let's stick with T for now
	array(const std::initializer_list<typename initializer_selector<T>::type> &init) {
		if (size) {
			actual_size = size;
		} else {
			actual_size = init.size();
		}
		m_arr = (T *)new char[sizeof(T) * actual_size];
		for (static_var<size_t> i = 0; i < actual_size; i++) {
			if (i < init.size())
				new (m_arr + i) T(*(init.begin() + i));
			else
				new (m_arr + i) T();
		}
	}


	void set_size(size_t new_size) {
		assert(size == 0 && "set_size should be only called for dyn_arr without size");
		assert(m_arr == nullptr && "set_size should be only called once");
		actual_size = new_size;
		m_arr = (T *)new char[sizeof(T) * actual_size];
		for (static_var<size_t> i = 0; i < actual_size; i++) {
			new (m_arr + i) T();
		}
	}

	template <typename T2, size_t N>
	void initialize_from_other(const array<T2, N> &other) {
		if (size) {
			actual_size = size;
		} else {
			actual_size = other.actual_size;
		}
		m_arr = (T*)new char[sizeof(T) * actual_size];
		for (static_var<size_t> i = 0; i < actual_size; i++) {
			if (i < other.actual_size)
				new (m_arr + i) T(other[i]);
			else
				new (m_arr + i) T();
		}
	}

	array(const array &other) {
		initialize_from_other(other);
	}
	template <typename T2, size_t N>
	array(const array<T2, N> &other) {
		initialize_from_other(other);
	}

	array &operator=(const array &other) = delete;

	T &operator[](size_t index) {
		assert(m_arr != nullptr && "Should call set_size for arrays that don't have a size");
		return m_arr[index];
	}
	const T &operator[](size_t index) const {
		assert(m_arr != nullptr && "Should call set_size for arrays that don't have a size");
		return m_arr[index];
	}

	~array() {
		if (m_arr) {
			for (static_var<size_t> i = 0; i < actual_size; i++) {
				m_arr[i].~T();
			}

			delete[](char *) m_arr;
		}
	}

	template <typename T2, size_t N>
	friend class array;
};

template <typename T, size_t N = 0> 
using dyn_arr = array<dyn_var<T>, N>;

template <typename T, size_t N = 0>
using arr = array<T, N>;

} // namespace builder

#endif

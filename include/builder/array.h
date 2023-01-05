#ifndef BUILDER_ARRAY_H
#define BUILDER_ARRAY_H
#include "builder/dyn_var.h"
#include "builder/static_var.h"

namespace builder {

template <typename T, size_t size = 0> 
class dyn_arr {
private:
	dyn_var<T>* m_arr = nullptr;
	size_t actual_size = 0;
public:

	dyn_arr() {
		if (size) {
			actual_size = size;
			m_arr = (dyn_var<T>*) new char[sizeof(dyn_var<T>) * actual_size];
			for (static_var<size_t> i = 0; i < actual_size; i++) {
				new (m_arr + i) dyn_var<T>();
			}
		}
	}
	dyn_arr(const std::initializer_list<builder>& init) {
		if (size) {
			actual_size = size;		
		} else {
			actual_size = init.size();
		}
		m_arr = (dyn_var<T>*) new char[sizeof(dyn_var<T>) * actual_size];
		for (static_var<size_t> i = 0; i < actual_size; i++) {
			if (i < init.size()) 
				new (m_arr + i) dyn_var<T>(*(init.begin() + i));
			else
				new (m_arr + i) dyn_var<T>();
		}
	}
	void set_size(size_t new_size) {
		assert(size == 0 && "set_size should be only called for dyn_arr without size");
		assert(m_arr == nullptr && "set_size should be only called once");
		actual_size = new_size;
		m_arr = (dyn_var<T>*) new char[sizeof(dyn_var<T>) * actual_size];
		for (static_var<size_t> i = 0; i < actual_size; i++) {
			new (m_arr + i) dyn_var<T>();
		}
	}

	template <typename T2, size_t N>
	void initialize_from_other(const dyn_arr<T2, N>& other) {
		if (size) {
			actual_size = size;
		} else {
			actual_size = other.actual_size;
		}
		m_arr = (dyn_var<T>*) new char[sizeof(dyn_var<T>) * actual_size];
		for (static_var<size_t> i = 0; i < actual_size; i++) {
			if (i < other.actual_size) 
				new (m_arr + i) dyn_var<T>(other[i]);
			else 
				new (m_arr + i) dyn_var<T>();
		}

	}

	dyn_arr (const dyn_arr& other) {
		initialize_from_other(other);	
	}	
	template <typename T2, size_t N>	
	dyn_arr(const dyn_arr<T2, N>& other) {
		initialize_from_other(other);
	}

	dyn_arr& operator= (const dyn_arr& other) = delete;

	dyn_var<T>& operator[](size_t index) {
		assert(m_arr != nullptr && "Should call set_size for arrays that don't have a size");
		return m_arr[index];
	}
	const dyn_var<T>& operator[] (size_t index) const {
		assert(m_arr != nullptr && "Should call set_size for arrays that don't have a size");
		return m_arr[index];
	}

	~dyn_arr() {
		if (m_arr) {
			for (static_var<size_t> i = 0; i < actual_size; i++) {
				m_arr[i].~dyn_var<T>();
			}
			
			delete[] (char*)m_arr;
		}
	}

	template <typename T2, size_t N> 
	friend class dyn_arr;
};

}

#endif

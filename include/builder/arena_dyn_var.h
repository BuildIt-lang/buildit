#ifndef ARENA_DYN_VAR_H
#define ARENA_DYN_VAR_H
#include <cstddef>

namespace builder {

using byte_t = unsigned char;
struct arena_list {
	std::vector<byte_t*> chunks;
	size_t used_objects = 0;
};
	
constexpr const int arena_objects_per_chunk = 64;

// Registry type for types that are allocatable
// Each global construct call assigns a unique id to each type
struct allocatable_type_registry {
	// Declaration for unique counter, definition is in cpp
	static int type_counter;
	
	// A function that can destroy a list of objects of a type
	using deleter_t = void(*)(arena_list*);

	// This has to be a pointer because we cannot guarantee order of invocation of constructors
	static std::vector<deleter_t> *type_deleters;
	
	int type_id;
	
	// Grab an ID and increment
	allocatable_type_registry(deleter_t deleter) {
		if (type_deleters == nullptr) {
			type_deleters = new std::vector<deleter_t>();
		}		
		type_deleters->push_back(deleter);	
		type_id = type_counter;
		type_counter++;
	}

	// Post global constructor, this returns the maximum 
	// type id. Make sure this is ONLY called from main
	static int get_max_type_id(void) {
		return type_counter;
	}
};

// This type is instantiated for each
// type that is allocated in the whole binary
template <typename T>
struct allocatable_type_manager {
	static allocatable_type_registry register_type;
	static void delete_objects(arena_list *list) {
		for (size_t i = 0; i < list->used_objects; i++) {
			int chunk_id = i / arena_objects_per_chunk;
			int chunk_offset = i % arena_objects_per_chunk;

			auto ptr_b = list->chunks[chunk_id] + sizeof(T) * chunk_offset;
			auto ptr = (T*)ptr_b;
			ptr->~T();	
		}
		list->used_objects = 0;
	}
};

template <typename T>
allocatable_type_registry allocatable_type_manager<T>::register_type(allocatable_type_manager<T>::delete_objects);

class dyn_var_arena {	
	// Arena contains a separate list for each type
	// indexed by a generated type id
	std::vector<arena_list> arena_lists;

	template <typename T>
	byte_t* grab_buffer() {
		// Get arena list index for this type
		int index = allocatable_type_manager<T>::register_type.type_id;	
		arena_list& list = arena_lists[index];
		// If list is full add another chunk	
		if (list.used_objects == list.chunks.size() * arena_objects_per_chunk) {	
			// Alignment and pointer and the end
			static_assert(alignof(T) <= alignof(std::max_align_t), 
				"Allocated type has higher alignment requirement that std::max_align_t" 
				"needs manual alignment");
			byte_t* new_chunk = new byte_t[sizeof(T) * arena_objects_per_chunk];
			list.chunks.push_back(new_chunk);		
		}
		int chunk_id = list.used_objects / arena_objects_per_chunk;
		int chunk_offset = list.used_objects % arena_objects_per_chunk;

		auto ptr = list.chunks[chunk_id] + sizeof(T) * chunk_offset;
		list.used_objects++;
		return ptr;
	}
	
public:

	dyn_var_arena() {
		arena_lists.resize(allocatable_type_registry::get_max_type_id());
	}

	template <typename T, typename...Args>
	T* allocate(Args&&...args) {
		auto ptr_b = grab_buffer<T>();
		auto ptr = new (ptr_b) T(std::forward<Args>(args)...);
		return ptr;
	}


	void reset_arena(void) {
		// For each type call the respective deleters on the type
		for (unsigned int i = 0; i < arena_lists.size(); i++) {
			if (arena_lists[i].used_objects > 0) {
				(*allocatable_type_registry::type_deleters)[i](&(arena_lists[i]));
			}
		}
	}
	
	~dyn_var_arena() {
		reset_arena();
		for (auto& a: arena_lists) {
			for (auto c: a.chunks) {
				delete[] c;
			}
		}	
	}
};

}

#endif

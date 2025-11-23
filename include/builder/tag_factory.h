#ifndef BUILDER_TAG_FACTORY_H
#define BUILDER_TAG_FACTORY_H

#include "util/tracer.h"

namespace builder {

class tag_factory {
	std::unordered_map<tracer::tag, tracer::tag_id> internal_map;
	tracer::tag_id next_id = 1;
public:
	tracer::tag_id create_tag_id (const tracer::tag& t) {
		auto it = internal_map.find(t);
		if (it != internal_map.end()) 
			return it->second;
		internal_map[t] = next_id++;
		return next_id - 1;
	}
};

}


#endif

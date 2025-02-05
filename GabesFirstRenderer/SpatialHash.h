#ifndef SPATIALHASH_H
#define SPATIALHASH_H
#include "glm/glm.hpp"

struct SpatialHash {

public:
	unsigned _index, _hash, _cellKey;

	SpatialHash() {
		_index = 0;
		_cellKey = 0;
		_hash = 0;
	}

	SpatialHash(unsigned index, unsigned hash, unsigned cellKey) {
		_index = index;
		_cellKey = cellKey;
		_hash = hash;
	}
};

#endif

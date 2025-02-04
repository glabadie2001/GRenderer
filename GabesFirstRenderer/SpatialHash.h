#ifndef SPATIALHASH_H
#define SPATIALHASH_H

struct SpatialHash {

public:
	int _index, _hash, _cellKey;

	SpatialHash() {
		_index = 0;
		_cellKey = 0;
		_hash = 0;
	}

	SpatialHash(int index, int hash, int cellKey) {
		_index = index;
		_cellKey = cellKey;
		_hash = hash;
	}
};

#endif

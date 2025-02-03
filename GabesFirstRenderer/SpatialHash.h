#ifndef SPATIALHASH_H
#define SPATIALHASH_H

#include <glm/vec2.hpp>

class SpatialHash {
	// Constants used for hashing
	static const unsigned hashK1 = 15823;
	static const unsigned hashK2 = 9737333;

public:
	int _index, _cellKey;
	static const glm::vec2* offsets2D;

	SpatialHash() {
		_index = 0;
		_cellKey = 0;
	}

	SpatialHash(int index, int cellKey) {
		_index = index;
		_cellKey = cellKey;
	}

	static glm::vec2 positionToCellCoord(const glm::vec2& point, float radius) {
		return glm::vec2(point.x / radius, point.y / radius);
	}

	static unsigned hashCell(const glm::vec2& point) {
		unsigned a = (unsigned)point.x * hashK1;
		unsigned b = (unsigned)point.y * hashK2;
		return a + b;
	}

	static unsigned keyFromHash(unsigned hash, unsigned count) {
		return hash % count;
	}
};

#endif

#pragma once
#include "SpatialHash.h"
#include "glm/vec2.hpp"
#include <cmath>

class SpatialHashMap
{
	int _count;

	// Constants used for hashing
	static const unsigned _hashK1 = 15823;
	static const unsigned _hashK2 = 9737333;

public:
	//index, hash, key
	glm::uvec4* _spatialIndices;
	unsigned* _spatialOffsets;

	SpatialHashMap(unsigned particleCount);

	static const glm::vec2* offsets2D;

	glm::uvec4* getMap() const;
	glm::uvec4 get(unsigned index) const;
	float* getCells() const;
	unsigned getStartIndex(unsigned index) const;
	unsigned count() const;
	void sort();
	void updateMap(const glm::vec2* points, unsigned count, float radius);

	~SpatialHashMap();

	/*static glm::vec2 positionToCellCoord(const glm::vec2& point, float radius) {
		glm::vec2 cellID = glm::vec2(std::floor(point.x / radius), std::floor(point.y / radius));
		return cellID;
	}*/

	static glm::vec2 positionToCellCoord(const glm::vec2& point, float radius) {
		return glm::vec2(
			std::floor(point.x / radius),
			std::floor(point.y / radius)
		);
	}

	static unsigned hashCell(const glm::vec2& cell) {
		unsigned a = (unsigned)cell.x * _hashK1;
		unsigned b = (unsigned)cell.y * _hashK2;
		return (a + b);

		// Convert cell coordinates to integers, maintaining precision
		//int32_t x = static_cast<int32_t>(std::floor(cell.x));
		//int32_t y = static_cast<int32_t>(std::floor(cell.y));

		//// Ensure positive values for bitwise operations
		//x = x >= 0 ? 2 * x : -2 * x - 1;
		//y = y >= 0 ? 2 * y : -2 * y - 1;

		//// Interleave bits using Morton encoding (Z-order curve)
		//unsigned hash = 0;
		//for (int i = 0; i < 16; ++i) {
		//	hash |= ((x & (1 << i)) << i) | ((y & (1 << i)) << (i + 1));
		//}

		//// Additional mixing for better distribution
		//hash ^= hash >> 13;
		//hash *= 0x85ebca6b;
		//hash ^= hash >> 17;
		//hash *= 0xc2b2ae35;

		//return hash;
	}

	static unsigned keyFromHash(unsigned hash, int count) {
		return hash % (unsigned)count;
	}
};


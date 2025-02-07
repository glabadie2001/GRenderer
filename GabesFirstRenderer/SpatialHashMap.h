#pragma once
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include <cmath>

class SpatialHashMap
{
	int _count;

	// Constants used for hashing
	static const unsigned _hashK1 = 15823;   // Large prime
	static const unsigned _hashK2 = 9737333;   // Large prime


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
	}

	static unsigned keyFromHash(unsigned hash, int count) {
		return hash % count;
	}
};


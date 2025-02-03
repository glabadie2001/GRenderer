#include "SpatialHash.h"
#include "glm/vec2.hpp"

const glm::vec2* SpatialHash::offsets2D = new glm::vec2[9]
{
	glm::vec2(-1, 1),
	glm::vec2(0, 1),
	glm::vec2(1, 1),
	glm::vec2(-1, 0),
	glm::vec2(0, 0),
	glm::vec2(1, 0),
	glm::vec2(-1, -1),
	glm::vec2(0, -1),
	glm::vec2(1, -1),
};
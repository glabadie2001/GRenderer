#ifndef PARTICLE_H
#define PARTICLE_H
#include "glm/vec2.hpp"
#include "Shader.h"
#include "SpatialHash.h"

class ParticleSystem
{
	float _screenWidth;
	float _screenHeight;

	unsigned int _vao;
	unsigned int _vertexBuffer;
	unsigned int _densityBuffer;
	//Each vertex is represented as three floats in a row
	float* _mappedBuffer;
	int _particleCount;
	int _vertices;
	const float _targetDensity = 1.2f;
	const float _pressureMultiplier = 25.0f;
	const float _smoothingRadius = 25.0f;

	SpatialHash** _spatialLookup;
	int* _startIndices;

	void resolveCollisions(glm::vec2* pos, glm::vec2* vel);
public:
	Shader* shader;
	glm::vec2* gravity;
	glm::vec2* positions;
	glm::vec2* velocities;
	float* densities;
	//Vector3* colors;
	ParticleSystem(int count, Shader* shader, float screenWidth, float screenHeight);
	int count() const;
	unsigned getVertices() const;
	unsigned getVBO() const;
	unsigned getDensityBuff() const;
	void simulate(), updateDensities(), updateSpatialLookup(glm::vec2* points, int count, float radius);
	float density(const glm::vec2& point) const;
	glm::vec2* pressure(int particleIndex) const;
	float densityToPressure(float density) const;

	void foreachPointInRadius(glm::vec2 point, void (*pred)(glm::vec2));

	~ParticleSystem();
};

#endif

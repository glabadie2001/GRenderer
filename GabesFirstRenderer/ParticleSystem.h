#ifndef PARTICLE_H
#define PARTICLE_H
#include "glm/vec2.hpp"
#include "Shader.h"
#include "SpatialHashMap.h"
#include <functional>

class ParticleSystem
{
	float _screenWidth;
	float _screenHeight;

	SpatialHashMap* _spatialHash;

	unsigned int _vao;
	unsigned int _vertexBuffer;
	unsigned int _densityBuffer;
	unsigned int _gridBuffer;
	unsigned int _velBuffer;

	int _particleCount;
	int _vertices;
	const float _targetDensity = 55.0f;
	const float _pressureMultiplier = 500.0;
	const float _nearPressureMultiplier = 18.0f;
	const float _smoothingRadius = 50.0f;

	int* _startIndices;

	void resolveCollisions(glm::vec2* pos, glm::vec2* vel);
public:
	Shader* shader;
	glm::vec2* gravity;
	glm::vec2* positions;
	glm::vec2* predictedPositions;
	glm::vec2* velocities;
	float* densities;
	float* nearDensities;
	//Vector3* colors;
	ParticleSystem(int count, Shader* shader, float screenWidth, float screenHeight);
	int count() const;
	unsigned getVertices() const;
	unsigned getVBO() const;
	unsigned getDensityBuff() const;
	void simulate(float deltaTime), updateDensities();
	float calcDensityInfluence(int index, int neighborIndex);
	glm::vec2 calcPressureInfluence(int index, int neighborIndex);
	float density(int particleIndex);
	glm::vec2* pressure(int particleIndex);
	float densityToPressure(float density) const, nearDensityToPressure(float density) const;
	glm::vec2 externalForces(int particleIndex);

	void foreachPointInRadius(int, const std::function<void(ParticleSystem*, int neighborIndex, float sqrDst)>& pred);

	~ParticleSystem();
};

#endif

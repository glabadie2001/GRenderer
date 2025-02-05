#ifndef PARTICLE_H
#define PARTICLE_H
#include "glm/vec2.hpp"
#include "Shader.h"
#include "ComputeShader.h"
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
	ComputeShader* densityCompute;
	ComputeShader* pressureCompute;

	int _particleCount;
	int _vertices;
	const float _targetDensity = 0.05f;
	const float _pressureMultiplier = 0.05f;
	const float _nearPressureMultiplier = 0.01f;
	const float _smoothingRadius = 200.0f;

	int* _startIndices;

	void resolveCollisions(glm::vec2* pos, glm::vec2* vel);
public:
	const float PI = 3.14159265358979323846f;
	const float SpikyPow3ScalingFactor = 10 / (PI * std::powf(_smoothingRadius, 5));
	const float SpikyPow2ScalingFactor = 6 / (PI * std::powf(_smoothingRadius, 5));
	const float SpikyPow3DerivativeScalingFactor = 30 / (std::powf(_smoothingRadius, 5) * PI);
	const float SpikyPow2DerivativeScalingFactor = 12 / (std::powf(_smoothingRadius, 4) * PI);
	float NearDensityKernel(float dst, float radius)
	{
		if (dst < radius)
		{
			float v = radius - dst;
			return v * v * v * SpikyPow3ScalingFactor;
		}
		return 0;
	}

	float DensityKernel(float dst, float radius)
	{
		if (dst < radius)
		{
			float v = radius - dst;
			return v * v * SpikyPow2ScalingFactor;
		}
		return 0;
	}

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
	float density(int particleIndex);
	glm::vec2* pressure(int particleIndex);
	float densityToPressure(float density) const, nearDensityToPressure(float density) const;
	glm::vec2 externalForces(int particleIndex);

	void foreachPointInRadius(int, const std::function<void(ParticleSystem*, int neighborIndex, float sqrDst)>& pred);

	~ParticleSystem();
};

#endif

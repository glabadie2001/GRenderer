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
	const float _targetDensity = 1.0f;
	const float _pressureMultiplier = 100.0f;
	const float _nearPressureMultiplier = 10.1f;
	const float _smoothingRadius = 50.0f;

	int* _startIndices;

	void resolveCollisions(glm::vec2* pos, glm::vec2* vel);
public:
	const float PI = 3.14159265358979323846f;
	const float SpikyPow3ScalingFactor = 10 / (PI * std::powf(_smoothingRadius, 5));
	const float SpikyPow2ScalingFactor = 6 / (PI * std::powf(_smoothingRadius, 4));
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
	glm::vec2 gravity = glm::vec2(0, 0);
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
	void simulate(float deltaTime);
	glm::vec2 externalForces(int particleIndex);

	void densityKernel(float deltaTime), pressureKernel(float deltaTime);

	const float getTargetDensity() const {
		return _targetDensity;
	}

	// Helper function to calculate aligned offset
	size_t getArrayStride(size_t elementSize, size_t elementAlignment, size_t count) {
		// First align the element size to its required alignment
		size_t alignedElementSize = (elementSize + elementAlignment - 1) & ~(elementAlignment - 1);
		// Then multiply by count
		return alignedElementSize * count;
	}

	size_t calculateBufferOffsets(size_t particleCount, size_t& positionsOffset, size_t& indicesOffset) {
		// SpatialOffsets (uint array): 4-byte aligned elements
		size_t offsetsSize = getArrayStride(sizeof(unsigned), 4, particleCount);

		// PredictedPositions (vec2 array): 8-byte aligned elements
		positionsOffset = offsetsSize;
		size_t positionsSize = getArrayStride(sizeof(glm::vec2), 8, particleCount);

		// SpatialIndices (uvec4 array): 16-byte aligned elements
		indicesOffset = positionsOffset + positionsSize;
		size_t indicesSize = getArrayStride(sizeof(glm::uvec4), 16, particleCount);

		return indicesOffset + indicesSize;
	}

	~ParticleSystem();
};

#endif

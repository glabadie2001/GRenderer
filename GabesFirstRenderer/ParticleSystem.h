#ifndef PARTICLE_H
#define PARTICLE_H
#include "glm/vec2.hpp"
#include "Shader.h"
#include "ComputeShader.h"
#include "SpatialHashMap.h"
#include <functional>
#include <glm/mat4x4.hpp>

class ParticleSystem
{
	float _screenWidth;
	float _screenHeight;

	float _prevScreenWidth;
	float _prevScreenHeight;
	glm::vec2 _windowPosition;
	glm::mat4 _projectionMatrix;
	void updateProjectionMatrix();

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
	const float _targetDensity = 0.01f;
	const float _pressureMultiplier = 100.0f;
	const float _nearPressureMultiplier = 100.1f;
	const float _smoothingRadius = 25.0f;

	int* _startIndices;

	void resolveCollisions(glm::vec2* pos, glm::vec2* vel);
public:
	const float PI = 3.14159265358979323846f;
	const float SpikyPow3ScalingFactor = 10 / (PI * std::powf(_smoothingRadius, 5));
	const float SpikyPow2ScalingFactor = 6 / (PI * std::powf(_smoothingRadius, 4));
	const float SpikyPow3DerivativeScalingFactor = 30 / (std::powf(_smoothingRadius, 5) * PI);
	const float SpikyPow2DerivativeScalingFactor = 12 / (std::powf(_smoothingRadius, 4) * PI);

	Shader* shader;
	glm::vec2 gravity = glm::vec2(0, -100);
	glm::vec2* positions;
	glm::vec2* predictedPositions;
	glm::vec2* velocities;
	float* densities;
	float* nearDensities;
	//Vector3* colors;
	ParticleSystem(int count, Shader* shader, float screenWidth, float screenHeight, float screenX, float screenY);
	int count() const;
	unsigned getVertices() const;
	unsigned getVBO() const;
	unsigned getDensityBuff() const;
	void simulate(float deltaTime);
	glm::vec2 externalForces(int particleIndex);

	void densityKernel(float deltaTime), pressureKernel(float deltaTime);

	void setWindowPosition(float x, float y) {
		_windowPosition = glm::vec2(x, y);
		updateProjectionMatrix();
	}

	void updateScreenSize(float width, float height) {
		shader->use();
		shader->setVec2("screenSize", glm::vec2(width, height));
		updateProjectionMatrix();
		// Calculate the change in screen dimensions
		float deltaWidth = width - _prevScreenWidth;
		float deltaHeight = height - _prevScreenHeight;

		// Calculate velocities for the "moving walls"
		// Using small timestep to simulate wall movement
		const float resizeTimeStep = 1.0f / 60.0f;
		glm::vec2 wallVelocity(deltaWidth / resizeTimeStep, deltaHeight / resizeTimeStep);

		// Update particles based on wall movement
		for (int i = 0; i < _particleCount; i++) {
			// Handle right wall movement
			if (deltaWidth < 0 && positions[i].x > width) {
				// Apply an impulse based on how far the wall has moved
				float penetration = positions[i].x - width;
				velocities[i].x = wallVelocity.x * 0.8f; // Scale factor for smoother interaction
				positions[i].x = width - penetration * 0.1f; // Push particle slightly inside
			}

			// Handle bottom wall movement
			if (deltaHeight < 0 && positions[i].y > height) {
				float penetration = positions[i].y - height;
				velocities[i].y = wallVelocity.y * 0.8f;
				positions[i].y = height - penetration * 0.1f;
			}

			// Ensure particles stay within bounds
			resolveCollisions(&positions[i], &velocities[i]);

			// Update predicted positions for next simulation step
			predictedPositions[i] = positions[i] + velocities[i] * (1.0f / 120.0f);
		}
	}

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

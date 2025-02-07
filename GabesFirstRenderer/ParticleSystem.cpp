#include "ParticleSystem.h"
#include "glm/vec2.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "utils.h"
#include "SpatialHashMap.h"
#include "ComputeShader.h"
#include <chrono>
#include "BufferLayout.h"

const extern float Poly6ScalingFactor = 1.0f;

static float ViscosityKernel(float dst, float radius)
{
	if (dst < radius)
	{
		float v = radius * radius - dst * dst;
		return v * v * v * Poly6ScalingFactor;
	}
	return 0;
}

ParticleSystem::ParticleSystem(int count, Shader* shader, float screenWidth, float screenHeight, float screenX, float screenY) {
	this->shader = shader;
	_particleCount = count;
	_screenWidth = screenWidth;
	_screenHeight = screenHeight;
	_windowPosition = glm::vec2(screenX, screenY);

	_spatialHash = new SpatialHashMap(_particleCount);

	// Adjust gravity for screen space (example value for 600px height screen)
	positions = new glm::vec2[_particleCount];
	predictedPositions = new glm::vec2[_particleCount];
	velocities = new glm::vec2[_particleCount];
	densities = new float[_particleCount];
	nearDensities = new float[_particleCount];
	
	densityCompute = new ComputeShader("DensityKernel.comp", 100000 + _particleCount * sizeof(glm::vec2));
	pressureCompute = new ComputeShader("PressureKernel.comp", 140000 + _particleCount * sizeof(glm::vec4));

	srand(0);
	
	int i = 0;
	glm::vec2 spacing = glm::vec2(500, 500);

	glm::vec2 spawnCenter = glm::vec2(_screenWidth / 2, _screenHeight / 2);

	int numX = (int)std::ceil(std::sqrt(spacing.x / spacing.y * _particleCount + (spacing.x - spacing.y) * (spacing.x - spacing.y) / (4 * spacing.y * spacing.y)) - (spacing.x - spacing.y) / (2 * spacing.y));
	int numY = (int)std::ceil(_particleCount / (float)numX);
	// When setting initial particle positions:
	for (int y = 0; y < numY; y++) {
		for (int x = 0; x < numX; x++) {
			if (i >= _particleCount) break;

			float tx = numX <= 1 ? 0.5f : x / (numX - 1.0f);
			float ty = numY <= 1 ? 0.5f : y / (numY - 1.0f);

			// Position relative to window
			glm::vec2 relativePos = glm::vec2(
				(tx - 0.5f) * spacing.x + spawnCenter.x,
				(ty - 0.5f) * spacing.y + spawnCenter.y
			);

			// Convert to screen space
			positions[i] = relativePos + _windowPosition;

			// Add jitter in screen space
			float angle = (rand() % 360) * 3.14f * 2;
			glm::vec2 dir = glm::vec2(std::cos(angle), std::sin(angle));
			glm::vec2 jitter = dir * 0.025f * ((float)(rand() % 360) - 0.5f);
			positions[i] += jitter;

			i++;
		}
	}
	/*for (int y = 0; y < numY; y++)
	{
		for (int x = 0; x < numX; x++)
		{
			if (i >= _particleCount) break;

			float tx = numX <= 1 ? 0.5f : x / (numX - 1.0f);
			float ty = numY <= 1 ? 0.5f : y / (numY - 1.0f);

			float angle = (rand() % 360) * 3.14f * 2;
			glm::vec2 dir = glm::vec2(std::cos(angle), std::sin(angle));
			glm::vec2 jitter = dir * 0.025f * ((float)(rand() % 360) - 0.5f);
			positions[i] = glm::vec2((tx - 0.5f) * spacing.x, (ty - 0.5f) * spacing.y);
			positions[i].x += jitter.x + spawnCenter.x;
			positions[i].y += jitter.y + spawnCenter.y;

			positions[i].x = std::max(0.0f, positions[i].x);
			positions[i].y = std::max(0.0f, positions[i].y);
			positions[i].x = std::min(_screenWidth, positions[i].x);
			positions[i].y = std::min(_screenHeight, positions[i].y);


			i++;
		}
	}*/

	for (i = 0; i < _particleCount; i++) {
		//Random initialization
		//positions[i] = glm::vec2(random_float(0.0, _screenWidth), random_float(0.0f, _screenHeight));
		predictedPositions[i] = glm::vec2(positions[i]);
		velocities[i] = glm::vec2(0.0f, 0.0f);
	}

	_spatialHash->updateMap(positions, _particleCount, _smoothingRadius);

	// Create and bind vertex array object
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// Create and bind vertex buffer
	glGenBuffers(1, &_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);

	// Initialize buffer with data, specifying dynamic usage
	glBufferData(GL_ARRAY_BUFFER, _particleCount * sizeof(glm::vec2), positions, GL_DYNAMIC_DRAW);
	// Set up vertex attributes for the positions
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &_densityBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _densityBuffer);
	glBufferData(GL_ARRAY_BUFFER, _particleCount * sizeof(float), densities, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &_gridBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _gridBuffer);
	float* cellValues = _spatialHash->getCells();
	glBufferData(GL_ARRAY_BUFFER, _particleCount * sizeof(float), cellValues, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);
	delete cellValues;

	glGenBuffers(1, &_velBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _velBuffer);
	glBufferData(GL_ARRAY_BUFFER, _particleCount * sizeof(glm::vec2), velocities, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(3);

	//Initialize density buffers
	BufferLayout densityInLayout;
	densityInLayout.addElement(sizeof(unsigned), 4, _particleCount, "spatialOffsets");
	densityInLayout.addElement(sizeof(glm::vec2), 8, _particleCount, "predictedPositions");
	densityInLayout.addElement(sizeof(glm::uvec4), 16, _particleCount, "spatialIndices");
	densityCompute->inputSSBO->setLayout(densityInLayout);

	BufferLayout densityOutLayout;
	densityOutLayout.addElement(sizeof(float), 4, _particleCount, "densities");
	densityOutLayout.addElement(sizeof(float), 4, _particleCount, "nearDensities");
	densityCompute->outputSSBO->setLayout(densityOutLayout);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, densityCompute->_ID
	densityCompute->use();
	//Initialize density uniforms
	glUniform1f(glGetUniformLocation(densityCompute->_ID, "SpikyPow2ScalingFactor"), SpikyPow2ScalingFactor);
	glUniform1f(glGetUniformLocation(densityCompute->_ID, "SpikyPow3ScalingFactor"), SpikyPow3ScalingFactor);
	glUniform1f(glGetUniformLocation(densityCompute->_ID, "smoothingRadius"), _smoothingRadius);
	glUniform1ui(glGetUniformLocation(densityCompute->_ID, "numParticles"), _particleCount);

	densityKernel(0.01);

	//Initialize pressure buffers
	BufferLayout pressureInLayout;
	pressureInLayout.addElement(sizeof(glm::vec2), 8, _particleCount, "velocities");
	pressureInLayout.addElement(sizeof(float), 4, _particleCount, "densities");
	pressureInLayout.addElement(sizeof(float), 4, _particleCount, "nearDensities");
	pressureInLayout.addElement(sizeof(unsigned), 4, _particleCount, "spatialOffsets");
	pressureInLayout.addElement(sizeof(glm::vec2), 8, _particleCount, "predictedPositions");
	pressureInLayout.addElement(sizeof(glm::uvec4), 16, _particleCount, "spatialIndices");
	pressureCompute->inputSSBO->setLayout(pressureInLayout);

	BufferLayout pressureOutLayout;
	pressureOutLayout.addElement(sizeof(glm::vec2), 8, _particleCount, "velocities");
	pressureCompute->outputSSBO->setLayout(pressureOutLayout);

	pressureCompute->use();
	//Initialize pressure uniforms
	glUniform1f(glGetUniformLocation(pressureCompute->_ID, "nearPressureMultiplier"), _nearPressureMultiplier);
	glUniform1f(glGetUniformLocation(pressureCompute->_ID, "pressureMultiplier"), _pressureMultiplier);
	glUniform1f(glGetUniformLocation(pressureCompute->_ID, "targetDensity"), _targetDensity);
	glUniform1f(glGetUniformLocation(pressureCompute->_ID, "SpikyPow2DerivativeScalingFactor"), SpikyPow2DerivativeScalingFactor);
	glUniform1f(glGetUniformLocation(pressureCompute->_ID, "SpikyPow3DerivativeScalingFactor"), SpikyPow3DerivativeScalingFactor);
	glUniform1f(glGetUniformLocation(pressureCompute->_ID, "smoothingRadius"), _smoothingRadius);
	glUniform1ui(glGetUniformLocation(pressureCompute->_ID, "numParticles"), _particleCount);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	setWindowPosition(screenX, screenY);

	std::cout << "Particle System Initialized with:" << std::endl;
	std::cout << "Screen dimensions: " << _screenWidth << "x" << _screenHeight << std::endl;
	std::cout << "Particle count: " << _particleCount << std::endl;
}

int ParticleSystem::count() const {
	return _particleCount;
}

unsigned int ParticleSystem::getVertices() const {
	return _vao;
}

unsigned int ParticleSystem::getVBO() const {
	return _vertexBuffer;
}

unsigned int ParticleSystem::getDensityBuff() const {
	return _densityBuffer;
}

void ParticleSystem::updateProjectionMatrix() {
	// Create view matrix that transforms from screen space to window space
	glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f),
		glm::vec3(-_windowPosition.x, -_windowPosition.y, 0.0f));

	// Create orthographic projection for window space
	_projectionMatrix = glm::ortho(0.0f, _screenWidth, 0.0f, _screenHeight);

	// Combine projection and view matrices
	glm::mat4 combined = _projectionMatrix * viewMatrix;

	// Update shader
	shader->use();
	shader->setMat4("projection", combined);
}

glm::vec2 ParticleSystem::externalForces(int particleIndex) {
	// Gravity
	glm::vec2 gravityAccel = gravity;

	return gravityAccel;
}

void ParticleSystem::densityKernel(float deltaTime) {
	densityCompute->use();

	densityCompute->inputSSBO->write(_spatialHash->_spatialOffsets, _particleCount * sizeof(unsigned), densityCompute->inputSSBO->getOffset("spatialOffsets"));
	densityCompute->inputSSBO->write(predictedPositions, _particleCount * sizeof(glm::vec2), densityCompute->inputSSBO->getOffset("predictedPositions"));
	densityCompute->inputSSBO->write(_spatialHash->_spatialIndices, _particleCount * sizeof(glm::uvec4), densityCompute->inputSSBO->getOffset("spatialIndices"));

	glUniform1f(glGetUniformLocation(densityCompute->_ID, "deltaTime"), deltaTime);

	densityCompute->bind();
	glDispatchCompute(count(), 1, 1);
	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	delete[] densities;
	delete[] nearDensities;
	densities = (float*)densityCompute->outputSSBO->read(sizeof(float) * count());
	nearDensities = (float*)densityCompute->outputSSBO->read(sizeof(float) * count(), sizeof(float) * count());
}

void ParticleSystem::pressureKernel(float deltaTime) {
	pressureCompute->use();

	// Write data to the buffer
	pressureCompute->inputSSBO->write(velocities, count() * sizeof(glm::vec2), pressureCompute->inputSSBO->getOffset("velocities"));
	pressureCompute->inputSSBO->write(densities, count() * sizeof(float), pressureCompute->inputSSBO->getOffset("densities"));
	pressureCompute->inputSSBO->write(nearDensities, count() * sizeof(float), pressureCompute->inputSSBO->getOffset("nearDensities"));
	pressureCompute->inputSSBO->write(_spatialHash->_spatialOffsets, count() * sizeof(unsigned), pressureCompute->inputSSBO->getOffset("spatialOffsets"));
	pressureCompute->inputSSBO->write(predictedPositions, count() * sizeof(glm::vec2), pressureCompute->inputSSBO->getOffset("predictedPositions"));
	pressureCompute->inputSSBO->write(_spatialHash->_spatialIndices, count() * sizeof(glm::uvec4), pressureCompute->inputSSBO->getOffset("spatialIndices"));

	glUniform1f(glGetUniformLocation(pressureCompute->_ID, "deltaTime"), deltaTime);

	pressureCompute->bind();
	glDispatchCompute(count(), 1, 1);

	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	delete[] velocities;
	velocities = (glm::vec2*)pressureCompute->outputSSBO->read(sizeof(glm::vec2) * count());
}

void ParticleSystem::simulate(float deltaTime) {
	printErrors();
	int i;

	auto start = std::chrono::high_resolution_clock::now();
	//External Forces Kernel
	for (i = 0; i < count(); i++) {
		velocities[i] += externalForces(i) * deltaTime;

		const float predictionFactor = 1 / 120;
		//cout << velocities[i].x << ", " << velocities[i].y << endl;
		predictedPositions[i] = positions[i] + velocities[i] * predictionFactor;
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "External Forces: " << duration.count() << "ms" << std::endl;

	start = std::chrono::high_resolution_clock::now();
	//Spatial Hash Kernel
	_spatialHash->updateMap(predictedPositions, count(), _smoothingRadius);
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "Spatial Mapping: " << duration.count() << "ms" << std::endl;

	start = std::chrono::high_resolution_clock::now();
	//Density Kernel
	densityKernel(deltaTime);
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "Density: " << duration.count() << "ms" << std::endl;
	
	start = std::chrono::high_resolution_clock::now();

	//Pressure Kernel
	pressureKernel(deltaTime);
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "Pressure: " << duration.count() << "ms" << std::endl;

	//Viscosity Kernel

	start = std::chrono::high_resolution_clock::now();
	//Update Positions
	for (i = 0; i < count(); i++) {
		positions[i] +=	velocities[i] * (float)deltaTime;
		resolveCollisions(&(positions[i]), &(velocities[i]));
	}
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "Positions: " << duration.count() << "ms" << std::endl;

	/* WRITE TO BUFFER */
	glBindBuffer(GL_ARRAY_BUFFER, _gridBuffer);
	float* cellValues = _spatialHash->getCells();
	glBufferData(GL_ARRAY_BUFFER, _particleCount * sizeof(float), cellValues, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, _velBuffer);
	glBufferData(GL_ARRAY_BUFFER, _particleCount * sizeof(glm::vec2), velocities, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, _densityBuffer);
	glBufferData(GL_ARRAY_BUFFER, _particleCount * sizeof(float), densities, GL_DYNAMIC_DRAW);
	printErrors();
}

//TODO: offset collision detection by pixelRatio * particleRadius
void ParticleSystem::resolveCollisions(glm::vec2* pos, glm::vec2* vel) {
	const float damping = 0.95f;

	// Convert window bounds to screen space
	float leftBound = _windowPosition.x;
	float rightBound = _windowPosition.x + _screenWidth;
	float bottomBound = _windowPosition.y;
	float topBound = _windowPosition.y + _screenHeight;

	if (pos->x < leftBound) {
		pos->x = leftBound;
		vel->x *= -damping;
	}
	else if (pos->x > rightBound) {
		pos->x = rightBound;
		vel->x *= -damping;
	}

	if (pos->y < bottomBound) {
		pos->y = bottomBound;
		vel->y *= -damping;
	}
	else if (pos->y > topBound) {
		pos->y = topBound;
		vel->y *= -damping;
	}
}

ParticleSystem::~ParticleSystem() {
	delete[] positions;
	delete[] predictedPositions;
	delete[] velocities;
	delete[] densities;
	delete shader;
	delete densityCompute;
	glDeleteBuffers(1, &_vertexBuffer);
}

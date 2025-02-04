#include "ParticleSystem.h"
#include "glm/vec2.hpp"
#include "utils.h";
#include "SpatialHashMap.h"

const extern float Poly6ScalingFactor = 1.0f;
const extern float SpikyPow3ScalingFactor = 1.0f;
const extern float SpikyPow2ScalingFactor = 1.0f;
const extern float SpikyPow3DerivativeScalingFactor = 1.0f;
const extern float SpikyPow2DerivativeScalingFactor = 1.0f;

static float smoothing(float dst, float radius) {
	if (dst >= radius) return 0;

	float volume = (std::_Pi_val * std::powf(radius, 4)) / 6;
	return (radius - dst) * (radius - dst) / volume;
}

static float smoothingDeriv(float dst, float radius) {
	if (dst >= radius) return 0;

	float scale = 12.0f / (std::powf(radius, 4.0f) * (float)std::_Pi_val);
	return (dst - radius) * scale;
}

static float ViscosityKernel(float dst, float radius)
{
	if (dst < radius)
	{
		float v = radius * radius - dst * dst;
		return v * v * v * Poly6ScalingFactor;
	}
	return 0;
}

static float NearDensityKernel(float dst, float radius)
{
	if (dst < radius)
	{
		float v = radius - dst;
		return v * v * v * SpikyPow3ScalingFactor;
	}
	return 0;
}

static float DensityKernel(float dst, float radius)
{
	if (dst < radius)
	{
		float v = radius - dst;
		return v * v * SpikyPow2ScalingFactor;
	}
	return 0;
}

static float NearDensityDerivative(float dst, float radius)
{
	if (dst <= radius)
	{
		float v = radius - dst;
		return -v * v * SpikyPow3DerivativeScalingFactor;
	}
	return 0;
}

static float DensityDerivative(float dst, float radius)
{
	if (dst <= radius)
	{
		float v = radius - dst;
		return -v * SpikyPow2DerivativeScalingFactor;
	}
	return 0;
}

void ParticleSystem::foreachPointInRadius(int targetIndex, const std::function<void(ParticleSystem*, int, float)>& pred) {
	int i, j;
	glm::vec2 center = SpatialHashMap::positionToCellCoord(predictedPositions[targetIndex], _smoothingRadius);
	float sqrRadius = _smoothingRadius * _smoothingRadius;

	for (i = 0; i < 9; i++) {
		unsigned hash = SpatialHashMap::hashCell(center + SpatialHashMap::offsets2D[i]);
		unsigned key = SpatialHashMap::keyFromHash(hash, count());
		unsigned cellStartIndex = _spatialHash->getStartIndex(key);

		for (int neighborIndex = cellStartIndex; neighborIndex < count(); neighborIndex++) {
			SpatialHash* entry = _spatialHash->get(neighborIndex);
			if (entry->_cellKey != key) break;
			if (entry->_hash != hash) continue;
			//if (entry->_index == targetIndex) continue;

			int particleIndex = _spatialHash->get(neighborIndex)->_index;
			glm::vec2 diff = (predictedPositions[particleIndex] - predictedPositions[targetIndex]);
			float sqrDst = (diff.x * diff.x) + (diff.y * diff.y);

			if (sqrDst <= sqrRadius) {
				pred(this, neighborIndex, sqrDst);
			}
		}
	}
}

ParticleSystem::ParticleSystem(int count, Shader* shader, float screenWidth, float screenHeight) {
	this->shader = shader;
	_particleCount = count;
	_screenWidth = screenWidth;
	_screenHeight = screenHeight;

	_spatialHash = new SpatialHashMap(_particleCount);

	// Adjust gravity for screen space (example value for 600px height screen)
	gravity = new glm::vec2(0, 0);
	positions = new glm::vec2[_particleCount];
	predictedPositions = new glm::vec2[_particleCount];
	velocities = new glm::vec2[_particleCount];
	densities = new float[_particleCount];
	nearDensities = new float[_particleCount];
	
	srand(0);
	
	int i = 0;
	glm::vec2 spacing = glm::vec2(500, 500);

	glm::vec2 spawnCenter = glm::vec2(_screenWidth / 2, _screenHeight / 2);

	int numX = std::ceil(std::sqrt(spacing.x / spacing.y * _particleCount + (spacing.x - spacing.y) * (spacing.x - spacing.y) / (4 * spacing.y * spacing.y)) - (spacing.x - spacing.y) / (2 * spacing.y));
	int numY = std::ceil(_particleCount / (float)numX);

	for (int y = 0; y < numY; y++)
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

			i++;
		}
	}

	for (i = 0; i < _particleCount; i++) {
		//Random initialization
		//positions[i] = glm::vec2(random_float(0.0, _screenWidth), random_float(0.0f, _screenHeight));
		predictedPositions[i] = glm::vec2(positions[i]);
		velocities[i] = glm::vec2(0.0f, 0.0f);
	}

	_spatialHash->updateMap(positions, _particleCount, _smoothingRadius);

	updateDensities();

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

glm::vec2 ParticleSystem::externalForces(int particleIndex) {
	// Gravity
	glm::vec2 gravityAccel = *gravity;

	return gravityAccel;
}

float ParticleSystem::density(int particleIndex) {
	float density = 0;
	float nearDensity = 0;
	const float mass = 1;

	foreachPointInRadius(particleIndex, [&density, &nearDensity, &particleIndex](ParticleSystem* ps, int neighborIndex, float sqrDst) {
		float dst = std::sqrt(sqrDst);
		density += DensityKernel(dst, ps->_smoothingRadius);
		nearDensity += NearDensityKernel(dst, ps->_smoothingRadius);
	});

	return density;
}

glm::vec2* ParticleSystem::pressure(int particleIndex) {
	glm::vec2* pressureForce = new glm::vec2(0.0f, 0.0f);
	float density = densities[particleIndex];
	float nearDensity = nearDensities[particleIndex];
	float pressure = densityToPressure(density);
	float nearPressure = nearDensityToPressure(nearDensity);

	foreachPointInRadius(particleIndex, [pressureForce, &particleIndex, &density, &pressure, &nearPressure](ParticleSystem* ps, int neighborIndex, float sqrDst) {
		float dst = std::sqrt(sqrDst);
		glm::vec2 dir = (dst > 0) ? (ps->predictedPositions[neighborIndex] - ps->predictedPositions[particleIndex])/dst : glm::vec2(0.0f, 1.0f);

		float neighborDensity = ps->densities[neighborIndex];
		float neighborNearDensity = ps->nearDensities[neighborIndex];
		float neighborPressure = ps->densityToPressure(neighborDensity);
		float neighborNearPressure = ps->nearDensityToPressure(ps->nearDensities[neighborIndex]);

		float sharedPressure = (density + neighborDensity) * 0.5f;
		float sharedNearPressure = (nearPressure + neighborNearPressure) * 0.5f;

		glm::vec2 addend1 = dir * DensityDerivative(dst, ps->_smoothingRadius) * sharedPressure / neighborDensity;
		glm::vec2 addend2 = dir * NearDensityDerivative(dst, ps->_smoothingRadius) * sharedNearPressure / neighborNearDensity;

		pressureForce->x += addend1.x + addend2.x;
		pressureForce->y += addend1.y + addend2.y;
	});

	return pressureForce;
}

float ParticleSystem::densityToPressure(float density) const {
	float densityError = density - _targetDensity;
	float pressure = densityError * _pressureMultiplier;
	return pressure;
}

float ParticleSystem::nearDensityToPressure(float nearDensity) const {
	return _nearPressureMultiplier * nearDensity;
}

void ParticleSystem::simulate(float deltaTime) {
	int i;

	//External Forces Kernel
	for (i = 0; i < count(); i++) {
		velocities[i] += externalForces(i) * deltaTime;

		const float predictionFactor = 1 / 120;
		//cout << velocities[i].x << ", " << velocities[i].y << endl;
		predictedPositions[i] = positions[i] + velocities[i] * predictionFactor;
	}

	//Spatial Hash Kernel
	for (i = 0; i < count(); i++) {
		//Reset offset to out-of-bounds value
		_spatialHash->_spatialOffsets[i] = count();
		glm::vec2 cell = _spatialHash->positionToCellCoord(predictedPositions[i], _smoothingRadius);
		unsigned hash = _spatialHash->hashCell(cell);
		unsigned key = _spatialHash->keyFromHash(hash, count());
		
		if (_spatialHash->_spatialIndices[i] == nullptr) {
			_spatialHash->_spatialIndices[i] = new SpatialHash(i, hash, key);
		}
		else {
			_spatialHash->_spatialIndices[i]->_index = i;
			_spatialHash->_spatialIndices[i]->_hash = hash;
			_spatialHash->_spatialIndices[i]->_cellKey = key;
		}
	}

	//Sort
	_spatialHash->sort();
	
	//Set Offsets
	//Iterate through sorted spatialIndices and wait for cellKey changes to mark boundaries.
	for (i = 0; i < count(); i++) {
		int key = _spatialHash->_spatialIndices[i]->_cellKey;
		int keyPrev = (i == 0) ? count() : _spatialHash->_spatialIndices[i - 1]->_cellKey;

		//Update offset position when change found
		if (key != keyPrev)
			_spatialHash->_spatialOffsets[key] = i;
	}

	//Density Kernel
	for (i = 0; i < count(); i++) {
		densities[i] = density(i);
	}
	
	//Pressure Kernel
	for (i = 0; i < count(); i++) {
		glm::vec2* pressureForce = pressure(i);
		// F = ma, a = F/m
		glm::vec2 acc = *pressureForce / densities[i];

		delete pressureForce;
		velocities[i] += acc * deltaTime;
	}

	//Viscosity Kernel

	//Update Positions
	for (i = 0; i < count(); i++) {
		positions[i] +=	velocities[i] * (float)deltaTime;
		resolveCollisions(&(positions[i]), &(velocities[i]));
	}

	//_spatialHash->updateMap(predictedPositions, count(), _smoothingRadius);

	/* WRITE TO BUFFER */
	glBindBuffer(GL_ARRAY_BUFFER, _gridBuffer);
	float* cellValues = _spatialHash->getCells();
	glBufferData(GL_ARRAY_BUFFER, _particleCount * sizeof(float), cellValues, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, _velBuffer);
	glBufferData(GL_ARRAY_BUFFER, _particleCount * sizeof(glm::vec2), velocities, GL_DYNAMIC_DRAW);

}

glm::vec2 ParticleSystem::calcPressureInfluence(int currIndex, int neighborIndex) {
	glm::vec2 offset = (positions[neighborIndex] - positions[currIndex]);
	float dst = std::sqrtf((offset.x * offset.x) + (offset.y * offset.y));

	glm::vec2 dir = (dst == 0) ? offset / 0.1f : offset / dst;
	float slope = smoothingDeriv(dst, _smoothingRadius);

	float density = densities[currIndex];
	float sharedPressure = (densityToPressure(density) + densityToPressure(densities[neighborIndex])) / 2;
	float mass = 1.0f;

	glm::vec2 addend = dir * sharedPressure * slope * mass / density;

	return addend;
}


float ParticleSystem::calcDensityInfluence(int index, int neighborIndex) {
	glm::vec2 diff = (positions[index] - positions[neighborIndex]);
	float dst = std::sqrtf(diff.x * diff.x + diff.y * diff.y);
	float influence = smoothing(_smoothingRadius, dst);
	//TODO: Mass?
	return influence;
}

void ParticleSystem::updateDensities() {
	int i;

	for (i = 0; i < count(); i++) {
		densities[i] = density(i);
	}
}

//TODO: offset collision detection by pixelRatio * particleRadius
void ParticleSystem::resolveCollisions(glm::vec2* pos, glm::vec2* vel) {
	const float damping = 0.95f;

	if (pos->x < 0) {
		pos->x = 0;
		vel->x *= -damping;
	}
	else if (pos->x > _screenWidth) {
		pos->x = _screenWidth * sgn(pos->x);
		vel->x *= -damping;
	}

	if (pos->y < 0) {
		pos->y = 0;
		vel->y *= -damping;
	}
	else if (pos->y > _screenHeight) {
		pos->y = _screenHeight * sgn(pos->y);
		vel->y *= -damping;
	}
}

ParticleSystem::~ParticleSystem() {
	delete[] positions;
	delete[] predictedPositions;
	delete[] velocities;
	delete[] densities;
	delete gravity;
	delete shader;
	glDeleteBuffers(1, &_vertexBuffer);
}

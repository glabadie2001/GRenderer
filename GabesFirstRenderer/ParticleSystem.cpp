#include "ParticleSystem.h"
#include "glm/vec2.hpp"
#include "utils.h";
#include "SpatialHash.h"

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

void ParticleSystem::foreachPointInRadius(glm::vec2 point, void (*pred)(glm::vec2)) {
	int i;
	glm::vec2 center = SpatialHash::positionToCellCoord(point, _smoothingRadius);
	float sqrRadius = _smoothingRadius * _smoothingRadius;

	for (i = 0; i < 9; i++) {
		unsigned hash = SpatialHash::hashCell(center + SpatialHash::offsets2D[i]);
		unsigned key = SpatialHash::keyFromHash(hash, count());
		unsigned currentIndex = _spatialLookup[key]->_index;

		while (currentIndex < count()) {

		}
	}
}

ParticleSystem::ParticleSystem(int count, Shader* shader, float screenWidth, float screenHeight) {
	this->shader = shader;
	_particleCount = count;
	_screenWidth = screenWidth;
	_screenHeight = screenHeight;

	// Adjust gravity for screen space (example value for 600px height screen)
	gravity = new glm::vec2(0, -0.05f);
	positions = new glm::vec2[count];
	velocities = new glm::vec2[count];
	densities = new float[count];
	
	_spatialLookup = new SpatialHash*[count];
	_startIndices = new int[count];

	srand(0);

	for (int i = 0; i < _particleCount; i++) {
		positions[i] = glm::vec2(random_float(0.0, _screenWidth), random_float(0.0f, _screenHeight));
		velocities[i] = glm::vec2(0.0f, 0.0f);
	}

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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &_densityBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _densityBuffer);
	glBufferData(GL_ARRAY_BUFFER, _particleCount * sizeof(float), densities, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	std::cout << "Particle System Initialized with:" << std::endl;
	std::cout << "Screen dimensions: " << _screenWidth << "x" << _screenHeight << std::endl;
	std::cout << "Particle count: " << _particleCount << std::endl;
}

float ParticleSystem::density(const glm::vec2& point) const {
	int i;
	float density = 0;
	const float mass = 1;

	//TODO: optimize to only look at particles inside smoothing radius
	for (i = 0; i < count(); i++) {
		glm::vec2 diff = (positions[i] - point);
		float dst = std::sqrtf(diff.x * diff.x + diff.y * diff.y);
		float influence = smoothing(_smoothingRadius, dst);
		density += mass * influence;
	}

	return density;
}

glm::vec2* ParticleSystem::pressure(int particleIndex) const {
	int i;

	glm::vec2* pressureForce = new glm::vec2(0.0f, 0.0f);

	for (i = 0; i < count(); i++) {
		if (particleIndex == i) continue;

		glm::vec2 offset = (positions[i] - positions[particleIndex]);
		float dst = std::sqrtf((offset.x * offset.x) + (offset.y * offset.y));

		glm::vec2 dir = (dst == 0) ? offset / 0.1f : offset / dst;
		float slope = smoothingDeriv(dst, _smoothingRadius);
		float density = densities[i];
		float sharedPressure = (densityToPressure(density) + densityToPressure(densities[particleIndex])) / 2;
		float mass = 1.0f;

		glm::vec2 addend = dir * sharedPressure * slope * mass / density;

		pressureForce->x += addend.x;
		pressureForce->y += addend.y;

		//std::cout << -densityToPressure(density) << ", " << slope << std::endl;

		//std::cout << dst << ", " << dir.x << ", " << slope << ", " << density << ", " << addend.x << ", " << addend.y << std::endl;
	}
	return pressureForce;
}

float ParticleSystem::densityToPressure(float density) const {
	float densityError = density - _targetDensity;
	float pressure = densityError * _pressureMultiplier;
	return pressure;
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

void ParticleSystem::simulate() {
	int i;

	for (i = 0; i < count(); i++) {
		velocities[i] += *gravity;
		densities[i] = density(positions[i]);
	}

	for (i = 0; i < count(); i++) {
		glm::vec2* pressureForce = pressure(i);
		// F = ma, a = F/m
		glm::vec2 acc = *pressureForce / densities[i];

		delete pressureForce;
		velocities[i] = acc;
	}

	for (i = 0; i < count(); i++) {
		positions[i] += velocities[i];
		resolveCollisions(&(positions[i]), &(velocities[i]));
	}
}

void ParticleSystem::updateDensities() {
	int i;

	for (i = 0; i < count(); i++) {
		densities[i] = density(positions[i]);
	}
}

void ParticleSystem::updateSpatialLookup(glm::vec2* points, int count, float radius) {
	int i;

	for (i = 0; i < count; i++) {
		glm::vec2 cellCoords = SpatialHash::positionToCellCoord(points[i], radius);
		unsigned cellKey = SpatialHash::keyFromHash(SpatialHash::hashCell(cellCoords), this->count());
		_spatialLookup[i] = new SpatialHash(i, cellKey);
		_startIndices[i] = UINT16_MAX;
	}

	std::sort(_spatialLookup[0], _spatialLookup[this->count() - 1], [](SpatialHash a, SpatialHash b) {
		return a._cellKey > b._cellKey;
	});

	for (i = 0; i < count; i++) {
		unsigned key = _spatialLookup[i]->_cellKey;
		unsigned keyPrev = i == 0 ? UINT16_MAX : _spatialLookup[i - 1]->_cellKey;
		if (key != keyPrev) {
			_startIndices[key] = i;
		}
	}
}

//TODO: offset collision detection by pixelRatio * particleRadius
void ParticleSystem::resolveCollisions(glm::vec2* pos, glm::vec2* vel) {
	const float damping = 0.8f;

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
	delete[] velocities;
	delete[] densities;
	delete gravity;
	delete shader;
	glDeleteBuffers(1, &_vertexBuffer);
}

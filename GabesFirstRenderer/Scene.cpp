#include "Scene.h"
#include "glad/glad.h"
#include "iostream"
#include <vector>

void Scene::add(Mesh* mesh) {
	_meshes.push_back(mesh);
}

void Scene::add(ParticleSystem* ps) {
	_particleSystems.push_back(ps);
}

std::vector<Mesh*> Scene::getMeshes() const {
	return _meshes;
}

std::vector<ParticleSystem*> Scene::getParticleSystems() const {
	return _particleSystems;
}

void Scene::update(float deltaTime) {
	int i;
	for (i = 0; i < _particleSystems.size(); i++) {
		_particleSystems[i]->simulate(deltaTime);
	}
}

Scene::~Scene() {
	int i;
	for (i = 0; i < _meshes.size(); i++) {
		delete _meshes[i];
	}

	for (i = 0; i < _particleSystems.size(); i++) {
		delete _particleSystems[i];
	}
}
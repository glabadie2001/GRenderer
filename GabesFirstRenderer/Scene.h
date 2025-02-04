#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "glad/glad.h"
#include "Mesh.h"
#include "ParticleSystem.h"

class Scene {
	std::vector<Mesh*> _meshes;
	std::vector<ParticleSystem*> _particleSystems;

public:
	void add(Mesh* mesh);
	void add(ParticleSystem* ps);
	void update(float deltaTime);

	std::vector<Mesh*> getMeshes() const;
	std::vector<ParticleSystem*> getParticleSystems() const;

	~Scene();
};

#endif
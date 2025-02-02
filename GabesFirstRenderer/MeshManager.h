#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include <vector>
#include "glad/glad.h"
#include "Mesh.h"

class MeshManager {
	std::vector<Mesh*> _vaos;

public:
	void Add(Mesh* mesh);
	void RenderAll();
};

#endif
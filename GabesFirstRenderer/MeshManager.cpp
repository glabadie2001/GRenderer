#include "MeshManager.h"
#include "glad/glad.h"
#include "iostream";

void MeshManager::Add(Mesh* mesh) {
	_vaos.push_back(mesh);
}

void MeshManager::RenderAll() {
	int i;

	for (i = 0; i < _vaos.size(); i++) {
		_vaos[i]->Draw();
	}
}
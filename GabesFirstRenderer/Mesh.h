#ifndef MESH_H
#define MESH_H

#include "Shader.h"

class Mesh {
	//VAO in OpenGL
	unsigned int _vertices;
	int _vertCount;
	//EBO in OpenGL
	unsigned int _indices;
	int _indCount;

	unsigned int _vbo;

public:
	Shader* shader;
	Mesh(float* vertices, int count, Shader* shader);
	Mesh(float* vertices, int vCount, unsigned int* indices, int iCount, Shader* shader);
	~Mesh();

	unsigned int getVertices() const;
	unsigned int getIndices() const;
	int getVertCount() const;
	int getIndCount() const;
};

#endif
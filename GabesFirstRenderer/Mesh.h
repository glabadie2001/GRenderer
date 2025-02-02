#ifndef MESH_H
#define MESH_H

class Mesh {
	//VAO in OpenGL
	unsigned int _vertices;
	int _vertCount;
	//EBO in OpenGL
	unsigned int _indices;
	int _indCount;

	unsigned int _vbo;

public:
	Mesh(float* vertices, int count);
	Mesh(float* vertices, int vCount, unsigned int* indices, int iCount);
	void Draw();
	~Mesh();
};

#endif
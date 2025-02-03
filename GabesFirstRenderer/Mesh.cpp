#include "Mesh.h"
#include "glad/glad.h"

Mesh::Mesh(float* vertices, int byteCount, Shader* shader) {
	this->shader = shader;
	_vertCount = byteCount / (3 * sizeof(float)); // Assuming 3 floats per vertex

	glGenVertexArrays(1, &_vertices);
	glGenBuffers(1, &_vbo);

	// ..:: Initialization code (done once (unless your object frequently changes)) :: ..
	// 1. bind Vertex Array Object
	glBindVertexArray(_vertices);
	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, byteCount, vertices, GL_STATIC_DRAW);
	// 3. then set our vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);
}

Mesh::Mesh(float* vertices, int vByteCount, unsigned int* indices, int iByteCount, Shader* shader): Mesh(vertices, vByteCount, shader) {
	_indCount = iByteCount / sizeof(unsigned int); // Number of indices

	glBindVertexArray(_vertices);
	glGenBuffers(1, &_indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iByteCount, indices, GL_STATIC_DRAW);
	glBindVertexArray(0);
}

int Mesh::getIndCount() const {
	return _indCount;
}

unsigned int Mesh::getIndices() const {
	return _indices;
}

int Mesh::getVertCount() const {
	return _vertCount;
}

unsigned int Mesh::getVertices() const {
	return _vertices;
}

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &_vertices);
	glDeleteBuffers(1, &_vbo);
	if (_indCount > 0) {
		glDeleteBuffers(1, &_indices);
	}
}
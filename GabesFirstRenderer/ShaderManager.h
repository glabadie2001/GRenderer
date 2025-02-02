#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <vector>
#include "Shader.h"

class ShaderManager {
	std::vector<Shader*> _shaders;
	unsigned int _shaderProgram;

	Shader* Compile(const char* shaderText, int shaderType);

public:
	ShaderManager();
	void CompileAndAdd(const char* shaderText, int shaderType);
	unsigned int BuildAndLink();
	void CleanUpShaders();
};

#endif
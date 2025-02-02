#include "ShaderManager.h"
#include "Shader.h"
#include <glad/glad.h>
#include <iostream>
#include <chrono>

ShaderManager::ShaderManager() {}

/*
 * Takes in compiled shaders to be managed.
 */
void ShaderManager::CompileAndAdd(const char* shaderText, int shaderType) {
	Shader* shader = Compile(shaderText, shaderType);
	_shaders.push_back(shader);
}

void ShaderManager::CleanUpShaders() {
	int i;
	for (i = 0; i < _shaders.size(); i++) {
		glDeleteShader(_shaders[i]->data);
	}
}

Shader* ShaderManager::Compile(const char* shaderText, int shaderType) {
	auto start = std::chrono::high_resolution_clock::now();

	unsigned int shaderData;
	shaderData = glCreateShader(shaderType);
	glShaderSource(shaderData, 1, &shaderText, NULL);
	glCompileShader(shaderData);

	//TODO: abstract out timer
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	//Error checking
	int  success;
	char infoLog[512];
	glGetShaderiv(shaderData, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shaderData, 512, NULL, infoLog);
		std::cout << "Shader compilation failed in " << duration.count() << "ms\n" << infoLog << std::endl;
	}
	else {
		std::cout << "Shader compiled successfully in " << duration.count() << "ms" << std::endl;
	}

	return new Shader(shaderData, shaderType);
}

unsigned int ShaderManager::BuildAndLink() {
	int i, success;
	char infoLog[512];

	auto start = std::chrono::high_resolution_clock::now();
	_shaderProgram = glCreateProgram();

	for (i = 0; i < _shaders.size(); i++) {
		glAttachShader(_shaderProgram, _shaders[i]->data);
	}
	glLinkProgram(_shaderProgram);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	/*
	 * Error Checking
	 */
	glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
		std::cout << "Shader program linking failed in " << duration.count() << "ms\n" << infoLog << std::endl;
	}
	else {
		std::cout << "Shader program linked in " << duration.count() << "ms" << std::endl;
	}

	CleanUpShaders();

	return _shaderProgram;
}
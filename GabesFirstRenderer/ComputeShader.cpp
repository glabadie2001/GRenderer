#include "ComputeShader.h"
#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>

ComputeShader::ComputeShader(const char* path, size_t ioSize)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string computeCode;
    std::ifstream cShaderFile;
    // ensure ifstream objects can throw exceptions:
    cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        cShaderFile.open(path);
        std::stringstream cShaderStream;
        // read file's buffer contents into streams
        cShaderStream << cShaderFile.rdbuf();
        // close file handlers
        cShaderFile.close();
        // convert stream into string
        computeCode = cShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    const char* cShaderCode = computeCode.c_str();

    // 2. compile shaders
    unsigned int compute;
    // vertex shader
    compute = compile(cShaderCode, GL_COMPUTE_SHADER);

    // shader Program
    _ID = glCreateProgram();
    glAttachShader(_ID, compute);
    glLinkProgram(_ID);
    checkCompileErrors(_ID, "PROGRAM");

    inputSSBO = new Buffer(ioSize);
    outputSSBO = new Buffer(ioSize);

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(compute);

    glUseProgram(0);
}

unsigned int ComputeShader::compile(const char* shaderCode, int type) {
    unsigned int shader;

    auto start = std::chrono::high_resolution_clock::now();
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    checkCompileErrors(shader, "VERTEX", duration.count());

    return shader;
}

// activate the shader
// ------------------------------------------------------------------------
void ComputeShader::use()
{
    glUseProgram(_ID);
}

void ComputeShader::bind() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *inputSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, *inputSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *outputSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, *outputSSBO);
}

// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
void ComputeShader::checkCompileErrors(unsigned int shader, std::string type, int duration)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n--------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n-- -------------------------------------------------- - -- " << std::endl;
        }
    }

    if (duration >= 0) {
        std::cout << "Took " << duration << "ms" << std::endl;
    }
}
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

class Shader
{
    unsigned int compile(const char* shaderCode, int type);

public:
    // the program ID
    unsigned int _ID;

    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void use();
    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, glm::vec2 value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    void checkCompileErrors(unsigned int shader, std::string type, int duration = -1);
};

#endif
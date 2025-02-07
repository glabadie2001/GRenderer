#pragma once
#include <string>
#include "Buffer.h"

class ComputeShader
{
    unsigned int compile(const char* shaderCode, int type);

public:
    unsigned int _ID;

    Buffer* inputSSBO;
    Buffer* outputSSBO;

    ComputeShader(const char* path, size_t ioSize);
    ~ComputeShader() {
        delete inputSSBO;
        delete outputSSBO;
    }

    void bind();
    void use();
    void checkCompileErrors(unsigned shader, std::string type, int duration = -1);
};
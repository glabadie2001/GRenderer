#pragma once
#include <string>
class ComputeShader
{
    size_t _bufferSize = 0; // Track actual size
    unsigned int compile(const char* shaderCode, int type);

public:
    unsigned int ID;

    unsigned int inputSSBO;
    unsigned int outputSSBO;

    ComputeShader(const char* path);

    void use();
    void* read(size_t amount, size_t unitSize, size_t offset = 0);
    void write(void* dataIn, size_t size, size_t offset = 0);
    void allocate(size_t totalSize);
    void checkCompileErrors(unsigned int shader, std::string type, int duration = -1);
};
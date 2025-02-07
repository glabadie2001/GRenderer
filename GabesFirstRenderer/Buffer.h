#pragma once
#include <assert.h>
#include <iostream>
#include <glad/glad.h>
#include "BufferLayout.h"

class Buffer {
	unsigned _ID;
	size_t _size;
    BufferLayout _layout;

public:

    Buffer() {
        glGenBuffers(1, &_ID);
    }

    Buffer(size_t inputSize): Buffer() {
        allocate(inputSize);
        _size = inputSize;
    }

    void setLayout(const BufferLayout& layout) {
        _layout = layout;
        _layout.calculateOffsets();
        allocate(_layout.getTotalSize());
    }

    int getOffset(std::string key) {
        return _layout.getOffset(key);
    }

    operator unsigned() const { return _ID; }

    void allocate(size_t totalSize) {
        if (totalSize == _size) return;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, totalSize, nullptr, GL_DYNAMIC_COPY);
        _size = totalSize;

        GLint bufferSize = 0;
        glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &bufferSize);

        assert(totalSize == bufferSize);
    }

    void* read(size_t byteCount, size_t offset = 0) {
        if (!_size) return nullptr;

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        void* output = new char[byteCount];

        // Bind and read
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ID);

        void* buff = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, byteCount, GL_MAP_READ_BIT);
        //Copy raw
        memcpy(output, buff, byteCount);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        return output;
    }

    void write(void* dataIn, size_t size, size_t offset = 0) {
        if (offset + size > _size) {
            throw std::runtime_error("Buffer overflow");
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ID);

        GLint bufferSize = 0;
        glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &bufferSize);
        assert(_size == bufferSize);

        //Access buffer as binary
        char* buff = (char*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        //Copy raw
        memcpy(buff, dataIn, size);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    }
};
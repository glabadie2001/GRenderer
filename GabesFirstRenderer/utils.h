#ifndef UTILS_H
#define UTILS_H
#include <random>
#include <iostream>
#include <glad/glad.h>
using namespace std;

template <typename T>
int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

inline float random_float(float min, float max) {
	return min + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min);
}

inline void printErrors() {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error: 0x" << std::hex << err << std::endl;
    }
}

#endif
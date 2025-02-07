#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// Describes a single buffer element
struct BufferElement {
    size_t elementSize;     // Size of individual element
    size_t alignment;       // Required alignment
    size_t count;          // Number of elements
    std::string name;      // Name for reference

    BufferElement(size_t _size, size_t align, size_t cnt, const std::string& n)
        : elementSize(_size), alignment(align), count(cnt), name(n) {}
};
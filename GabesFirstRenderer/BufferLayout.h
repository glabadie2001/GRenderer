#pragma once
#include <stdexcept>
#include "BufferElement.h"

class BufferLayout {
private:
    std::vector<BufferElement> elements;
    std::unordered_map<std::string, size_t> offsetMap;
    size_t totalSize;

    // Helper function to calculate aligned offset (kept from your original code)
    size_t getArrayStride(size_t elementSize, size_t elementAlignment, size_t count) {
        size_t alignedElementSize = (elementSize + elementAlignment - 1) & ~(elementAlignment - 1);
        return alignedElementSize * count;
    }

public:
    BufferLayout() : totalSize(0) {}

    // Add a new buffer element
    void addElement(size_t elementSize, size_t alignment, size_t count, const std::string& name) {
        elements.emplace_back(elementSize, alignment, count, name);
    }

    // Calculate all offsets
    void calculateOffsets() {
        size_t currentOffset = 0;

        for (const auto& element : elements) {
            // Align the current offset to the element's required alignment
            currentOffset = (currentOffset + element.alignment - 1) & ~(element.alignment - 1);

            // Store the offset for this element
            offsetMap[element.name] = currentOffset;

            // Calculate size of this element's array and add to current offset
            currentOffset += getArrayStride(element.elementSize, element.alignment, element.count);
        }

        totalSize = currentOffset;
    }

    // Get offset for a named element
    size_t getOffset(const std::string& name) const {
        auto it = offsetMap.find(name);
        if (it != offsetMap.end()) {
            return it->second;
        }
        throw std::runtime_error("Element not found: " + name);
    }

    // Get total buffer size
    size_t getTotalSize() const {
        return totalSize;
    }
};
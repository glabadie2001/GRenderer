#include <chrono>
#include "SpatialHashMap.h"
#include "utils.h"

SpatialHashMap::SpatialHashMap(unsigned particleCount) {
	_count = particleCount;
    _spatialIndices = new glm::uvec4[_count];
	_spatialOffsets = new unsigned[_count];
}

const glm::vec2* SpatialHashMap::offsets2D = new glm::vec2[9] {
	glm::vec2(-1, 1),
	glm::vec2(0, 1),
	glm::vec2(1, 1),
	glm::vec2(-1, 0),
	glm::vec2(0, 0),
	glm::vec2(1, 0),
	glm::vec2(-1, -1),
	glm::vec2(0, -1),
	glm::vec2(1, -1),
};

glm::uvec4* SpatialHashMap::getMap() const {
    return _spatialIndices;
}

float* SpatialHashMap::getCells() const {
    int i;
    
    /*int j;
    glm::vec2 root = glm::vec2(1, 0);
    int* captures = new int[10];
    captures[0] = SpatialHashMap::hashCell(root);

    for (i = 0; i < 9; i++) {
        unsigned hash = SpatialHashMap::hashCell(root + SpatialHashMap::offsets2D[i]);
        captures[i + 1] = SpatialHashMap::keyFromHash(hash, count());
    }

    float* res = new float[count()];
    for (i = 0; i < count(); i++) {
        int swapped = 0;
        glm::uvec4 entry = get(i);
        for (j = 0; j < count(); j++) {
            if (entry[1] == captures[j]) {
                res[entry[0]] = (float)(entry[2]);
                swapped = 1;
                break;
            }
        }
        if (swapped) continue;
        res[entry[0]] = 0;
    }

    delete[] captures;*/

    float* res = new float[count()];
    for (i = 0; i < count(); i++) {
        int swapped = 0;
        glm::uvec4 entry = get(i);
        res[entry[0]] = (float)(entry[2]);
    }

    return res;
}

glm::uvec4 SpatialHashMap::get(unsigned index) const {
	assert(index < _count);

	return _spatialIndices[index];
}

unsigned SpatialHashMap::getStartIndex(unsigned index) const {
	return _spatialOffsets[index];
}

unsigned SpatialHashMap::count() const {
	return _count;
}

SpatialHashMap::~SpatialHashMap() {
    delete[] _spatialIndices;      // Then delete the array of pointers
    delete[] _spatialOffsets;      // Don't forget this one!
}

void merge(glm::uvec4* arr, int left, int mid, int right) {
    int n1 = mid - left;
    int n2 = right - mid;

    glm::uvec4* leftArr = new glm::uvec4[n1];
    glm::uvec4* rightArr = new glm::uvec4[n2];

    // Explicit component-wise copy
    for (int i = 0; i < n1; i++) {
        leftArr[i][0] = arr[left + i][0];
        leftArr[i][1] = arr[left + i][1];
        leftArr[i][2] = arr[left + i][2];
        leftArr[i][3] = arr[left + i][3];
    }

    for (int i = 0; i < n2; i++) {
        rightArr[i][0] = arr[mid + i][0];
        rightArr[i][1] = arr[mid + i][1];
        rightArr[i][2] = arr[mid + i][2];
        rightArr[i][3] = arr[mid + i][3];
    }

    int i = 0;
    int j = 0;
    int k = left;

    while (i < n1 && j < n2) {
        if (leftArr[i][2] <= rightArr[j][2]) {
            // Explicit component-wise copy
            arr[k][0] = leftArr[i][0];
            arr[k][1] = leftArr[i][1];
            arr[k][2] = leftArr[i][2];
            arr[k][3] = leftArr[i][3];
            i++;
        }
        else {
            // Explicit component-wise copy
            arr[k][0] = rightArr[j][0];
            arr[k][1] = rightArr[j][1];
            arr[k][2] = rightArr[j][2];
            arr[k][3] = rightArr[j][3];
            j++;
        }
        k++;
    }

    while (i < n1) {
        // Explicit component-wise copy
        arr[k][0] = leftArr[i][0];
        arr[k][1] = leftArr[i][1];
        arr[k][2] = leftArr[i][2];
        arr[k][3] = leftArr[i][3];
        i++;
        k++;
    }

    while (j < n2) {
        // Explicit component-wise copy
        arr[k][0] = rightArr[j][0];
        arr[k][1] = rightArr[j][1];
        arr[k][2] = rightArr[j][2];
        arr[k][3] = rightArr[j][3];
        j++;
        k++;
    }

    delete[] leftArr;
    delete[] rightArr;
}

void merge_sort(glm::uvec4* arr, int left, int right) {
    if (left < right - 1) {  // right - left > 1
        int mid = left + (right - left) / 2;

        // Sort first and second halves
        merge_sort(arr, left, mid);
        merge_sort(arr, mid, right);

        merge(arr, left, mid, right);
    }
}

void SpatialHashMap::sort() {
    merge_sort(_spatialIndices, 0, _count);
}



void SpatialHashMap::updateMap(const glm::vec2* points, unsigned count, float radius) {
    if (count > _count) {
        return;
    }

    for (unsigned i = 0; i < count; i++) {
        //Create
        glm::vec2 cellCoord = positionToCellCoord(points[i], radius);
        unsigned cellHash = hashCell(cellCoord);
        unsigned cellKey = keyFromHash(cellHash, _count);
        _spatialIndices[i] = glm::uvec4(i, cellHash, cellKey, 0);
        _spatialOffsets[i] = UINT_MAX;
    }

    auto start = std::chrono::high_resolution_clock::now();
    sort();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Sort: " << duration.count() << "ms" << std::endl;

    //Iterates through sorted indices
    for (unsigned i = 0; i < count; i++) {
        unsigned key = _spatialIndices[i][2];
        unsigned keyPrev = (i == 0) ? UINT_MAX : _spatialIndices[i - 1][2];
        if (key != keyPrev)
            _spatialOffsets[key] = i;
    }
}

void SpatialHashMap::warmMap(const glm::vec2* points, unsigned count, float radius) {
    if (count > _count) {
        return;
    }

    for (unsigned i = 0; i < count; i++) {
        //Create
        glm::vec2 cellCoord = positionToCellCoord(points[i], radius);
        unsigned cellHash = hashCell(cellCoord);
        unsigned cellKey = keyFromHash(cellHash, _count);
        _spatialIndices[i] = glm::uvec4(i, cellHash, cellKey, 0);
        _spatialOffsets[i] = UINT_MAX;
    }

    sort();

    //Iterates through sorted indices
    for (unsigned i = 0; i < count; i++) {
        unsigned key = _spatialIndices[i][2];
        unsigned keyPrev = (i == 0) ? UINT_MAX : _spatialIndices[i - 1][2];
        if (key != keyPrev)
            _spatialOffsets[key] = i;
    }
}

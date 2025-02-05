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
    /*
    int j;
    glm::vec2 root = glm::vec2(10, 10);
    int* captures = new int[10];
    captures[0] = SpatialHashMap::hashCell(root);

    for (i = 0; i < 9; i++) {
        unsigned hash = SpatialHashMap::hashCell(root + SpatialHashMap::offsets2D[i]);
        captures[i + 1] = SpatialHashMap::keyFromHash(hash, count());
    }

    float* res = new float[count()];
    for (i = 0; i < count(); i++) {
        int swapped = 0;
        SpatialHash* entry = get(i);
        for (j = 0; j < count(); j++) {
            if (entry->_cellKey == captures[j]) {
                res[entry->_index] = (float)(entry->_cellKey);
                swapped = 1;
                break;
            }
        }
        if (swapped) continue;
        res[entry->_index] = 0;
    }

    delete[] captures*/;

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

    // Create temporary arrays
    glm::uvec4* leftArr = new glm::uvec4 [n1];
    glm::uvec4* rightArr = new glm::uvec4 [n2];

    // Copy data to temporary arrays
    for (int i = 0; i < n1; i++)
        leftArr[i] = arr[left + i];
    for (int i = 0; i < n2; i++)
        rightArr[i] = arr[mid + i];

    // Merge the temporary arrays back
    int i = 0;    // Initial index of left subarray
    int j = 0;    // Initial index of right subarray
    int k = left; // Initial index of merged subarray

    while (i < n1 && j < n2) {
        if (leftArr[i][2] <= rightArr[j][2]) {
            arr[k] = leftArr[i];
            i++;
        }
        else {
            arr[k] = rightArr[j];
            j++;
        }
        k++;
    }

    // Copy remaining elements of leftArr[] if any
    while (i < n1) {
        arr[k] = leftArr[i];
        i++;
        k++;
    }

    // Copy remaining elements of rightArr[] if any
    while (j < n2) {
        arr[k] = rightArr[j];
        j++;
        k++;
    }

    // Free temporary arrays
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
        _spatialOffsets[i] = count;
    }

    sort();

    for (unsigned i = 0; i < count; i++) {
        unsigned key = _spatialIndices[i][2];
        if (key >= _count) continue;
        unsigned keyPrev = (i == 0) ? UINT_MAX : _spatialIndices[i - 1][2];
        if (key != keyPrev)
            _spatialOffsets[key] = i;
    }
}

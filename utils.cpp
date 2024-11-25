
#include <fstream>
#include <iostream>
#include "utils.h"


std::vector<size_t> getShuffledIndices(size_t count) {
    std::vector<size_t> indices(count);
    std::iota(indices.begin(), indices.end(), 0); // Fill with 0, 1, ..., count-1

    // Shuffle the indices
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    return indices;
}

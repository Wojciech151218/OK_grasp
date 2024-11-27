
#ifndef OK_GRASP_UTILS_H
#define OK_GRASP_UTILS_H

#include <numeric>
#include <random>
#include "Solution.h"
#define let const auto
typedef  std::pair<Solution, float> RCL_tuple  ;

template<typename T = size_t>
size_t count_empty_vectors(const std::vector<std::vector<T>> & vector_of_vectors) {
    auto emptyCount = 0;
    for (const auto& innerVec : vector_of_vectors) {
        if (innerVec.empty()) {
            ++emptyCount;
        }
    }
    return  emptyCount;
}
template<typename T= size_t>
void remove_empty_vectors(std::vector<std::vector<T> > &vector_of_vectors) {
    vector_of_vectors.erase(
            std::remove_if(vector_of_vectors.begin(), vector_of_vectors.end(),
                           [](const std::vector<T>& innerVec) { return innerVec.empty(); }),
            vector_of_vectors.end());
}
std::vector<size_t> getShuffledIndices(size_t count) ;


#endif //OK_GRASP_UTILS_H

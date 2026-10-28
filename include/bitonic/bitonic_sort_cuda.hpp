#pragma once

#include <vector>

namespace bitonic {

// Return the values in ascending order without modifying the input.
// Throws std::length_error if the padded input exceeds one CUDA thread block,
// and std::runtime_error if a CUDA runtime operation fails.
std::vector<int> sort_cuda(const std::vector<int>& values);

}  // namespace bitonic

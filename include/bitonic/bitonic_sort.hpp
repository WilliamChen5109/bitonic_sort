#pragma once

#include <vector>

namespace bitonic {

// Return the values in ascending order without modifying the input.
std::vector<int> sort_cpu(const std::vector<int>& values);

}  // namespace bitonic

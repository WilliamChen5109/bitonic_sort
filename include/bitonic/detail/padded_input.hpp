#pragma once

#include <climits>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

namespace bitonic {
namespace detail {

inline std::vector<int> pad_to_power_of_two(const std::vector<int>& values) {
    if (values.empty()) {
        return {};
    }

    std::size_t padded_size = 1;
    while (padded_size < values.size()) {
        if (padded_size > std::numeric_limits<std::size_t>::max() / 2) {
            throw std::length_error("input is too large to pad to a power of two");
        }
        padded_size *= 2;
    }

    std::vector<int> padded(values);
    padded.resize(padded_size, INT_MAX);
    return padded;
}

inline void remove_padding(std::vector<int>& values, std::size_t original_size) {
    values.resize(original_size);
}

}  // namespace detail
}  // namespace bitonic

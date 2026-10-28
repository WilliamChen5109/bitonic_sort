#include "bitonic/bitonic_sort.hpp"

#include "bitonic/detail/padded_input.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace bitonic {
namespace {

void compare_and_swap(std::vector<int>& values,
                      std::size_t left,
                      std::size_t right,
                      bool ascending) {
    if ((ascending && values[left] > values[right]) ||
        (!ascending && values[left] < values[right])) {
        std::swap(values[left], values[right]);
    }
}

void merge(std::vector<int>& values,
           std::size_t first,
           std::size_t count,
           bool ascending) {
    if (count <= 1) {
        return;
    }

    const std::size_t half = count / 2;
    for (std::size_t i = first; i < first + half; ++i) {
        compare_and_swap(values, i, i + half, ascending);
    }
    merge(values, first, half, ascending);
    merge(values, first + half, half, ascending);
}

void bitonic_sort(std::vector<int>& values,
                  std::size_t first,
                  std::size_t count,
                  bool ascending) {
    if (count <= 1) {
        return;
    }

    const std::size_t half = count / 2;
    bitonic_sort(values, first, half, true);
    bitonic_sort(values, first + half, half, false);
    merge(values, first, count, ascending);
}

}  // namespace

std::vector<int> sort_cpu(const std::vector<int>& values) {
    if (values.empty()) {
        return {};
    }

    std::vector<int> result = detail::pad_to_power_of_two(values);
    bitonic_sort(result, 0, result.size(), true);
    detail::remove_padding(result, values.size());
    return result;
}

}  // namespace bitonic

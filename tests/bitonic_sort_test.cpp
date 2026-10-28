#include "bitonic/bitonic_sort.hpp"

#include <algorithm>
#include <climits>
#include <iostream>
#include <random>
#include <vector>

namespace {

bool check_case(const std::vector<int>& input) {
    std::vector<int> expected = input;
    std::sort(expected.begin(), expected.end());
    const std::vector<int> actual = bitonic::sort_cpu(input);
    if (actual != expected) {
        std::cerr << "bitonic::sort_cpu returned an unexpected result\n";
        return false;
    }
    return true;
}

}  // namespace

int main() {
    const std::vector<std::vector<int>> cases = {
        {},
        {1},
        {2, 1},
        {30, 80, 3, 4, 89, 23, 56, 78, 3, 6, 15, 17, 46, 23, 12},
        {INT_MAX, -1, INT_MIN, INT_MAX, 0},
        {5, 5, 5, 5},
        {-3, -2, -1, 0, 1},
        {5, 4, 3, 2, 1, 0},
    };

    for (const auto& input : cases) {
        if (!check_case(input)) {
            return 1;
        }
    }
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> distribution(-10000, 10000);
    for (const std::size_t size : {3, 7, 8, 9, 31, 32, 33, 1023, 1024, 1025}) {
        std::vector<int> input(size);
        for (int& value : input) {
            value = distribution(generator);
        }
        if (!check_case(input)) {
            return 1;
        }
    }
    return 0;
}

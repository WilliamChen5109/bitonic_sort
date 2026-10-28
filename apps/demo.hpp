#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace bitonic {
namespace demo {

inline std::vector<int> read_values(int argc, char* argv[]) {
    if (argc == 1) {
        return {30, 80, 3, 4, 89, 23, 56, 78, 3, 6, 15, 17, 46, 23, 12};
    }

    std::vector<int> values;
    values.reserve(static_cast<std::size_t>(argc - 1));
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        std::size_t parsed = 0;
        int value;
        try {
            value = std::stoi(argument, &parsed);
        } catch (const std::invalid_argument&) {
            throw std::invalid_argument("invalid integer: " + argument);
        } catch (const std::out_of_range&) {
            throw std::out_of_range("integer out of range: " + argument);
        }
        if (parsed != argument.size()) {
            throw std::invalid_argument("invalid integer: " + argument);
        }
        values.push_back(value);
    }
    return values;
}

inline void print_values(const char* label, const std::vector<int>& values) {
    std::cout << label;
    for (const int value : values) {
        std::cout << value << ' ';
    }
    std::cout << '\n';
}

using SortFunction = std::vector<int> (*)(const std::vector<int>&);

inline int run(int argc, char* argv[], SortFunction sort) {
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        std::cout << "用法: " << argv[0] << " [整数 ...]\n"
                  << "不传参数时使用内置示例序列。\n";
        return 0;
    }
    try {
        const std::vector<int> values = read_values(argc, argv);
        print_values("原始序列: ", values);
        print_values("排序结果: ", sort(values));
    } catch (const std::exception& error) {
        std::cerr << "错误: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

}  // namespace demo
}  // namespace bitonic

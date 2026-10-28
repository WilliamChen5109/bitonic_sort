# Bitonic Sort

这是一个用 C++/CUDA 实现的双调排序（bitonic sort）示例项目，包含 CPU 递归实现和可选的 CUDA 实现。两种实现都支持命令行整数输入；未提供输入时会对原有的 15 个整数示例排序。

原来的 `bs_serial.cpp` 和 `bs_parallel.cu` 已拆分为算法库与独立的程序入口；公共输入处理只保留一份，可执行文件统一生成到构建目录。

## 项目结构

```text
.
├── apps/
│   ├── bitonic_cpu_main.cpp
│   ├── bitonic_cuda_main.cpp
│   └── demo.hpp
├── include/bitonic/
│   ├── bitonic_sort.hpp
│   ├── bitonic_sort_cuda.hpp
│   └── detail/padded_input.hpp
├── src/
│   ├── bitonic_sort.cpp
│   └── bitonic_sort_cuda.cu
├── tests/bitonic_sort_test.cpp
├── .gitignore
├── CMakeLists.txt
└── README.md
```

- `include/bitonic/`：公共排序接口，以及 `detail/` 下的内部填充辅助代码。
- `src/`：排序算法实现，不包含命令行输入输出。
- `apps/`：两个程序入口及共用的参数解析、示例数据和打印逻辑。
- `tests/`：将 CPU 排序结果与 `std::sort` 对比的正确性测试。
- `build/`：CMake 自动生成的构建文件与程序，不应提交到版本库。

## 算法实现

Bitonic sort 先递归构造一个先升序、再降序的 bitonic 序列，然后通过 bitonic merge 反复比较交换相距一半区间的元素，最终得到升序结果。递归实现便于在 CPU 上展示算法结构。

排序网络要求元素数量为 2 的幂。程序会在输入尾部补上 `INT_MAX`，排序完成后将结果长度恢复为原长度；输入中原有的 `INT_MAX` 也会被保留。排序接口不会修改输入，支持负数和重复值，空输入直接返回空结果。

CUDA 版本把一个完整的排序网络映射到单个 thread block：每个线程对应一个元素，在阶段 `k`、比较距离 `j` 下通过 `tid ^ j` 计算比较伙伴，通过 `(tid & k) == 0` 确定升降序。仅 `partner > tid` 的线程执行比较交换，避免重复写入，每轮结束时使用 `__syncthreads()` 同步。数据保存在 GPU 全局内存中，当前未使用共享内存优化。

该版本保留原程序的单 block 设计。输入补齐后的长度必须满足设备和 kernel 的线程数限制，常见上限为 1024 个元素；例如长度为 1025 的输入需要补齐到 2048，因而无法在该上限下执行。程序在启动 kernel 前检查限制，使用异常报告 CUDA 运行错误，并通过资源管理对象释放显存。仅增加 block 数量不能扩展这个实现，因为 `__syncthreads()` 只能同步同一个 block 内的线程。

设补齐后的长度为 `N`，CPU 比较次数为 `O(N log² N)`，递归栈为 `O(log N)`，结果缓冲区为 `O(N)`。CUDA 使用 `N` 个线程完成 `log₂N × (log₂N + 1) / 2` 轮比较交换，额外显存为 `O(N)`。这个项目用于说明算法与并行映射，尚未进行性能基准测试；小规模数据还会受到内存传输和 kernel 启动开销的影响。

## 构建

需要 CMake 3.16 或更新版本和 C++17 编译器。CUDA 构建还需要支持 C++14 的 CUDA Toolkit 及与其兼容的宿主编译器；当前 `.cu` 文件采用 C++14 标准，以兼容本项目的最低 CMake 版本。本项目已在 CMake 3.16.3、GCC 9.4 和 CUDA 11.8 下通过编译。

CMake 默认检测 CUDA 编译器；未检测到时只生成 CPU 目标，检测到但工具链配置损坏时仍可能报错，可修复工具链或关闭 CUDA 构建。

当前验证环境无法访问 GPU，CUDA 程序运行时报驱动错误，因此 GPU 上的排序结果尚未验证。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
(cd build && ctest --output-on-failure)
```

如需强制只构建 CPU 版本：

```bash
cmake -S . -B build-cpu -DBITONIC_ENABLE_CUDA=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-cpu --parallel
(cd build-cpu && ctest --output-on-failure)
```

上面的测试命令兼容 CMake 3.16。测试默认启用，覆盖空输入、单元素、重复值、负数、整数极值、已排序序列、逆序序列，以及 2 的幂两侧的固定随机样本。测试不依赖 GPU；当前未包含 GPU 自动测试。

| CMake 选项 | 默认值 | 作用 |
| --- | --- | --- |
| `BITONIC_ENABLE_CUDA` | `ON` | 检测 CUDA 并尝试构建 GPU 库及示例 |
| `BUILD_TESTING` | `ON` | 构建并注册 CPU 正确性测试 |

如果 CUDA 不在默认路径中，可在首次配置时添加 `-DCMAKE_CUDA_COMPILER=/实际路径/bin/nvcc`。编译 CUDA 程序本身不需要 GPU，运行它则需要可用的 NVIDIA GPU 和兼容驱动。

## 运行

```bash
./build/bitonic_cpu_demo
./build/bitonic_cpu_demo 9 1 7 3
./build/bitonic_cpu_demo --help
```

只构建 CPU 时，将命令中的 `build/` 改为 `build-cpu/`。

CUDA 编译器可用时还会生成：

```bash
./build/bitonic_cuda_demo
./build/bitonic_cuda_demo 9 1 7 3
```

两种程序都会打印原始序列和排序结果；无效整数或 CUDA 运行时错误会输出错误信息并以非零状态退出。

内置示例的排序结果为：

```text
3 3 4 6 12 15 17 23 23 30 46 56 78 80 89
```

## 在其他 CMake 目标中使用

在同一 CMake 工程中链接 `bitonic_cpu` 并包含 `bitonic/bitonic_sort.hpp`，然后调用：

```cpp
const auto result = bitonic::sort_cpu({9, 1, 7, 3});
```

CUDA 编译器可用且 CUDA 选项开启时，可链接 `bitonic_cuda`、包含 `bitonic/bitonic_sort_cuda.hpp` 并调用 `bitonic::sort_cuda`，接口同样接收 `const std::vector<int>&` 并返回排好序的新向量。

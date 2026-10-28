#include "bitonic/bitonic_sort_cuda.hpp"

#include "bitonic/detail/padded_input.hpp"

#include <cuda_runtime.h>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace bitonic {
namespace {

void check_cuda(cudaError_t status, const char* operation) {
    if (status != cudaSuccess) {
        throw std::runtime_error(std::string(operation) + ": " + cudaGetErrorString(status));
    }
}

class DeviceBuffer {
public:
    explicit DeviceBuffer(std::size_t bytes) {
        check_cuda(cudaMalloc(reinterpret_cast<void**>(&data_), bytes), "cudaMalloc");
    }

    ~DeviceBuffer() {
        if (data_ != nullptr) {
            cudaFree(data_);
        }
    }

    DeviceBuffer(const DeviceBuffer&) = delete;
    DeviceBuffer& operator=(const DeviceBuffer&) = delete;

    int* get() const { return data_; }

private:
    int* data_ = nullptr;
};

__global__ void bitonic_sort_kernel(int* values, int count) {
    const unsigned int index = threadIdx.x;

    for (int stage = 2; stage <= count; stage <<= 1) {
        for (int distance = stage >> 1; distance > 0; distance >>= 1) {
            const unsigned int partner = index ^ static_cast<unsigned int>(distance);
            if (partner > index) {
                const bool ascending = (index & static_cast<unsigned int>(stage)) == 0;
                if ((ascending && values[index] > values[partner]) ||
                    (!ascending && values[index] < values[partner])) {
                    const int temporary = values[index];
                    values[index] = values[partner];
                    values[partner] = temporary;
                }
            }
            __syncthreads();
        }
    }
}

}  // namespace

std::vector<int> sort_cuda(const std::vector<int>& values) {
    if (values.empty()) {
        return {};
    }

    std::vector<int> result = detail::pad_to_power_of_two(values);

    int device = 0;
    check_cuda(cudaGetDevice(&device), "cudaGetDevice");
    cudaDeviceProp properties{};
    check_cuda(cudaGetDeviceProperties(&properties, device), "cudaGetDeviceProperties");
    cudaFuncAttributes attributes{};
    check_cuda(cudaFuncGetAttributes(&attributes, bitonic_sort_kernel), "cudaFuncGetAttributes");
    const int max_threads = std::min(properties.maxThreadsDim[0],
                                    std::min(properties.maxThreadsPerBlock,
                                             attributes.maxThreadsPerBlock));
    if (result.size() > static_cast<std::size_t>(max_threads)) {
        throw std::length_error("CUDA bitonic sort currently supports at most " +
                                std::to_string(max_threads) +
                                " padded values (one thread block)");
    }

    const std::size_t bytes = result.size() * sizeof(int);
    DeviceBuffer device_values(bytes);
    check_cuda(cudaMemcpy(device_values.get(), result.data(), bytes, cudaMemcpyHostToDevice),
               "cudaMemcpy host to device");

    bitonic_sort_kernel<<<1, static_cast<unsigned int>(result.size())>>>(
        device_values.get(), static_cast<int>(result.size()));
    check_cuda(cudaGetLastError(), "bitonic sort kernel launch");
    check_cuda(cudaDeviceSynchronize(), "bitonic sort kernel execution");
    check_cuda(cudaMemcpy(result.data(), device_values.get(), bytes, cudaMemcpyDeviceToHost),
               "cudaMemcpy device to host");

    detail::remove_padding(result, values.size());
    return result;
}

}  // namespace bitonic

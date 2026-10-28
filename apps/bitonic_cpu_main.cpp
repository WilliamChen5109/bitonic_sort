#include "bitonic/bitonic_sort.hpp"
#include "demo.hpp"

int main(int argc, char* argv[]) {
    return bitonic::demo::run(argc, argv, bitonic::sort_cpu);
}

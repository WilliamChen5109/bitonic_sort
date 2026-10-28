cmake_minimum_required(VERSION 3.16)

project(bitonic_sort LANGUAGES CXX)

option(BITONIC_ENABLE_CUDA "Build the CUDA implementation when a CUDA compiler is available" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bitonic_cpu src/bitonic_sort.cpp)
target_include_directories(bitonic_cpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(bitonic_cpu PRIVATE
    "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wall;-Wextra;-Wpedantic>"
)

add_executable(bitonic_cpu_demo apps/bitonic_cpu_main.cpp)
target_link_libraries(bitonic_cpu_demo PRIVATE bitonic_cpu)

include(CTest)
if(BUILD_TESTING)
    add_executable(bitonic_cpu_tests tests/bitonic_sort_test.cpp)
    target_link_libraries(bitonic_cpu_tests PRIVATE bitonic_cpu)
    add_test(NAME bitonic_cpu_tests COMMAND bitonic_cpu_tests)
endif()

if(BITONIC_ENABLE_CUDA)
    include(CheckLanguage)
    check_language(CUDA)

    if(CMAKE_CUDA_COMPILER)
        enable_language(CUDA)
        set(CMAKE_CUDA_STANDARD 14)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)

        add_library(bitonic_cuda src/bitonic_sort_cuda.cu)
        target_include_directories(bitonic_cuda PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
        add_executable(bitonic_cuda_demo apps/bitonic_cuda_main.cpp)
        target_link_libraries(bitonic_cuda_demo PRIVATE bitonic_cuda)
    else()
        message(STATUS "CUDA compiler not found; only CPU targets will be built")
    endif()
endif()

cmake_minimum_required(VERSION 3.15)
project(StaticWordleSolver VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optional CUDA support
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_CUDA "Enable CUDA acceleration" OFF "CMAKE_CUDA_COMPILER" OFF)

if(ENABLE_CUDA)
    enable_language(CUDA)
    add_definitions(-DENABLE_CUDA)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Testing
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

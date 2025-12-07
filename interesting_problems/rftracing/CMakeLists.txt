cmake_minimum_required(VERSION 3.18)
project(CudaExample LANGUAGES CXX CUDA)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable CUDA language
enable_language(CUDA)

# Set CUDA architecture - try common ones, adjust based on your GPU
set(CMAKE_CUDA_ARCHITECTURES 61 70 75 80 86)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add executable
add_executable(cuda_example
    src/main.cpp
)

# Set CUDA properties
set_property(TARGET cuda_example PROPERTY CUDA_SEPARABLE_COMPILATION ON)

# Set output directory
set_target_properties(cuda_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
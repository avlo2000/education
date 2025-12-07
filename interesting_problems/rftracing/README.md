# CUDA Example Project

A simple CUDA demonstration project showing vector addition with performance comparison.

## Features

- CUDA kernel implementation (`.cu` file)
- Vector addition using GPU parallel processing
- Performance comparison between CUDA and CPU
- CUDA device information display
- Memory management and error handling
- CMake build system

## Requirements

- CUDA Toolkit (11.0 or later)
- CMake (3.18 or later)
- C++17 compatible compiler
- NVIDIA GPU with compute capability 6.1 or higher

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Running

```bash
./bin/rftracing
```

## Project Structure

- `src/main.cpp` - Host application entry point
- `src/vector_add.cu` - CUDA kernel implementation
- `include/vector_add.h` - CUDA function declarations
- `include/` - Header files
- `CMakeLists.txt` - CMake configuration

## Output

The program performs vector addition on 1 million elements and outputs:
- CUDA device information
- Execution times for both CUDA and CPU
- Performance speedup comparison
- Result verification
- Sample computation results

This serves as a foundation for more complex CUDA applications.
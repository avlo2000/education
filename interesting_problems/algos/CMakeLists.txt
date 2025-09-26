cmake_minimum_required(VERSION 3.16)
project(algos C CXX)

# Language standards
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_ASAN "Enable AddressSanitizer for test targets" ON)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer for test targets" ON)
option(ENABLE_TSAN "Enable ThreadSanitizer for test targets" OFF)

# Common warning flags (GNU/Clang)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wconversion -Wsign-conversion -Wshadow
        -Wundef -Wdouble-promotion
    )
endif()

# Convenience function to apply sanitizers only to given targets
function(enable_sanitizers TARGET_NAME)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        # ThreadSanitizer is generally incompatible with Address/Undefined sanitizers in same binary.
        if (ENABLE_TSAN)
            target_compile_options(${TARGET_NAME} PRIVATE -fsanitize=thread -fno-omit-frame-pointer -O1)
            target_link_options(${TARGET_NAME} PRIVATE -fsanitize=thread)
            return()
        endif()
        if (ENABLE_ASAN)
            target_compile_options(${TARGET_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer -O1)
            target_link_options(${TARGET_NAME} PRIVATE -fsanitize=address)
        endif()
        if (ENABLE_UBSAN)
            target_compile_options(${TARGET_NAME} PRIVATE -fsanitize=undefined)
            target_link_options(${TARGET_NAME} PRIVATE -fsanitize=undefined)
        endif()
    endif()
endfunction()

# Include current dir for headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# C test executables
add_executable(minsegtree_test minsegtree_test.c)
enable_sanitizers(minsegtree_test)

add_executable(sumsegtree_test sumsegtree_test.c)
enable_sanitizers(sumsegtree_test)

# Optional standalone algorithm demos (C files) if you want them as executables
# add_executable(fenwick_sum_tree fenwick_sum_tree.c)
# add_executable(rolling_hash rolling_hash.c)
# add_executable(lca lca.c)
# add_executable(fft_c fft.c)

# C++ test / demo executables
add_executable(stamped_buffer_test stamped_buffer_test.cpp)
enable_sanitizers(stamped_buffer_test)

add_executable(stamped_buffer_comprehensive_test stamped_buffer_comprehensive_test.cpp)
enable_sanitizers(stamped_buffer_comprehensive_test)

add_executable(stamped_buffer_multithreaded_test stamped_buffer_multithreaded_test.cpp)
enable_sanitizers(stamped_buffer_multithreaded_test)

add_executable(gd2_gauss_elim gd2_gauss_elim.cpp)
enable_sanitizers(gd2_gauss_elim)

# Threads for multithreaded tests
find_package(Threads REQUIRED)
target_link_libraries(stamped_buffer_multithreaded_test PRIVATE Threads::Threads)

# Enable CTest integration
include(CTest)
if (BUILD_TESTING)
    add_test(NAME minsegtree_test COMMAND minsegtree_test)
    add_test(NAME sumsegtree_test COMMAND sumsegtree_test)
    add_test(NAME stamped_buffer_basic COMMAND stamped_buffer_test)
    add_test(NAME stamped_buffer_comprehensive COMMAND stamped_buffer_comprehensive_test)
    add_test(NAME stamped_buffer_multithreaded COMMAND stamped_buffer_multithreaded_test)
    add_test(NAME gd2_gauss_elim COMMAND gd2_gauss_elim)
endif()

message(STATUS "Configured algos with tests. Enable/disable sanitizers via -DENABLE_ASAN=ON/OFF -DENABLE_UBSAN=ON/OFF")

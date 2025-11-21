#include "stamped_buffer_safe_but_slow.hpp"
#include <vector>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <random>

// Test utility functions
void test_assert(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "[PASS] " << test_name << std::endl;
    } else {
        std::cout << "[FAIL] " << test_name << std::endl;
        assert(false);
    }
}

// Test 1: Basic functionality with simple sequence
void test_basic_functionality() {
    std::cout << "\n=== Test Basic Functionality ===" << std::endl;
    
    StampedRingBuffer<int> buffer(5);
    
    // Test empty buffer
    test_assert(buffer.empty(), "Empty buffer check");
    test_assert(buffer.size() == 0, "Empty buffer size");
    
    // Add elements with increasing timestamps
    for (int i = 0; i < 3; ++i) {
        buffer.add({(uint64_t)(i * 10), i});
    }
    
    test_assert(buffer.size() == 3, "Buffer size after adding 3 elements");
    test_assert(!buffer.empty(), "Buffer not empty after adding elements");
    
    // Test binary search for exact matches
    auto result = buffer.binsearch(10);
    test_assert(result.ts == 10 && result.elem == 1, "Exact match search for ts=10");
    
    result = buffer.binsearch(0);
    test_assert(result.ts == 0 && result.elem == 0, "Exact match search for ts=0");
    
    result = buffer.binsearch(20);
    test_assert(result.ts == 20 && result.elem == 2, "Exact match search for ts=20");
}

// Test 2: Binary search for closest values
void test_closest_timestamp_search() {
    std::cout << "\n=== Test Closest Timestamp Search ===" << std::endl;
    
    StampedRingBuffer<char> buffer(10);
    
    // Add elements: ts = 10, 20, 30, 40, 50
    for (int i = 1; i <= 5; ++i) {
        buffer.add({(uint64_t)(i * 10), 'A' + i - 1});
    }
    
    // Test searching for values between existing timestamps
    auto result = buffer.binsearch(15); // Between 10 and 20
    test_assert(result.ts == 10 || result.ts == 20, "Search for ts=15 returns closest");
    
    result = buffer.binsearch(25); // Between 20 and 30
    test_assert(result.ts == 20 || result.ts == 30, "Search for ts=25 returns closest");
    
    result = buffer.binsearch(35); // Between 30 and 40
    test_assert(result.ts == 30 || result.ts == 40, "Search for ts=35 returns closest");
    
    // Test edge cases - before first element
    result = buffer.binsearch(5); // Before ts=10
    test_assert(result.ts == 10, "Search for ts=5 returns first element");
    
    // Test edge cases - after last element
    result = buffer.binsearch(55); // After ts=50
    test_assert(result.ts == 50, "Search for ts=55 returns last element");
}

// Test 3: Ring buffer wraparound behavior
void test_ring_buffer_wraparound() {
    std::cout << "\n=== Test Ring Buffer Wraparound ===" << std::endl;
    
    StampedRingBuffer<int> buffer(3); // Small buffer to force wraparound
    
    // Fill buffer completely
    for (int i = 0; i < 3; ++i) {
        buffer.add({(uint64_t)(i * 10), i});
    }
    test_assert(buffer.size() == 3, "Buffer full");
    
    // Add more elements to trigger wraparound
    for (int i = 3; i < 6; ++i) {
        buffer.add({(uint64_t)(i * 10), i});
    }
    
    test_assert(buffer.size() == 3, "Buffer size remains 3 after wraparound");
    
    // The buffer should now contain elements with ts = 30, 40, 50
    auto result = buffer.binsearch(40);
    test_assert(result.ts == 40, "Search in wrapped buffer for ts=40");
    
    result = buffer.binsearch(35); // Between 30 and 40
    test_assert(result.ts == 30 || result.ts == 40, "Search in wrapped buffer for ts=35");
    
    result = buffer.binsearch(55); // After last element
    test_assert(result.ts == 50, "Search in wrapped buffer for ts=55");
}

// Test 4: Single element buffer
void test_single_element() {
    std::cout << "\n=== Test Single Element Buffer ===" << std::endl;
    
    StampedRingBuffer<double> buffer(5);
    buffer.add({100, 3.14});
    
    test_assert(buffer.size() == 1, "Single element buffer size");
    
    auto result = buffer.binsearch(100);
    test_assert(result.ts == 100 && result.elem == 3.14, "Exact match in single element buffer");
    
    result = buffer.binsearch(50); // Before the element
    test_assert(result.ts == 100, "Search before single element");
    
    result = buffer.binsearch(150); // After the element
    test_assert(result.ts == 100, "Search after single element");
}

// Test 5: Two element buffer edge cases
void test_two_elements() {
    std::cout << "\n=== Test Two Element Buffer ===" << std::endl;
    
    StampedRingBuffer<int> buffer(5);
    buffer.add({10, 1});
    buffer.add({30, 3});
    
    test_assert(buffer.size() == 2, "Two element buffer size");
    
    // Test exact matches
    auto result = buffer.binsearch(10);
    test_assert(result.ts == 10, "Exact match first element");
    
    result = buffer.binsearch(30);
    test_assert(result.ts == 30, "Exact match second element");
    
    // Test in-between value
    result = buffer.binsearch(20);
    test_assert(result.ts == 10 || result.ts == 30, "Search between two elements");
    
    // Test edge cases
    result = buffer.binsearch(5);
    test_assert(result.ts == 10, "Search before first element");
    
    result = buffer.binsearch(35);
    test_assert(result.ts == 30, "Search after last element");
}

// Test 6: Duplicate timestamps
void test_duplicate_timestamps() {
    std::cout << "\n=== Test Duplicate Timestamps ===" << std::endl;
    
    StampedRingBuffer<int> buffer(5);
    buffer.add({10, 1});
    buffer.add({20, 2});
    buffer.add({20, 22}); // Duplicate timestamp
    buffer.add({30, 3});
    
    auto result = buffer.binsearch(20);
    test_assert(result.ts == 20, "Search for duplicate timestamp");
    // Either element with ts=20 is acceptable
    test_assert(result.elem == 2 || result.elem == 22, "Correct element for duplicate timestamp");
}

// Test 7: Large buffer with random access patterns
void test_large_buffer_random() {
    std::cout << "\n=== Test Large Buffer Random Access ===" << std::endl;
    
    StampedRingBuffer<int> buffer(100);
    std::vector<uint64_t> timestamps;
    
    // Add elements with increasing timestamps
    for (int i = 0; i < 50; ++i) {
        uint64_t ts = i * 100;
        timestamps.push_back(ts);
        buffer.add({ts, i});
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 4900);
    
    // Test random searches
    for (int i = 0; i < 20; ++i) {
        uint64_t search_ts = dis(gen);
        auto result = buffer.binsearch(search_ts);
        
        // Find the closest timestamp manually for verification
        auto it = std::lower_bound(timestamps.begin(), timestamps.end(), search_ts);
        uint64_t expected_ts;
        
        if (it == timestamps.end()) {
            expected_ts = timestamps.back();
        } else if (it == timestamps.begin()) {
            expected_ts = timestamps.front();
        } else {
            // Choose the closer one
            uint64_t upper = *it;
            uint64_t lower = *(it - 1);
            expected_ts = (search_ts - lower <= upper - search_ts) ? lower : upper;
        }
        
        // The result should be reasonably close to expected
        bool close_match = (result.ts == expected_ts) || 
                          (std::abs((int64_t)result.ts - (int64_t)search_ts) <= 
                           std::abs((int64_t)expected_ts - (int64_t)search_ts) + 100);
        
        test_assert(close_match, "Random search " + std::to_string(i) + " for ts=" + std::to_string(search_ts));
    }
}

// Test 8: Buffer wraparound with complex patterns
void test_complex_wraparound() {
    std::cout << "\n=== Test Complex Wraparound Patterns ===" << std::endl;
    
    StampedRingBuffer<int> buffer(4);
    
    // Fill and wrap multiple times
    for (int i = 0; i < 10; ++i) {
        buffer.add({(uint64_t)(i * 10), i});
    }
    
    // Buffer should contain elements 6, 7, 8, 9 with timestamps 60, 70, 80, 90
    test_assert(buffer.size() == 4, "Complex wraparound buffer size");
    
    auto result = buffer.binsearch(75);
    test_assert(result.ts == 70 || result.ts == 80, "Search in complex wrapped buffer");
    
    result = buffer.binsearch(55); // Before all elements
    test_assert(result.ts == 60, "Search before all elements in wrapped buffer");
    
    result = buffer.binsearch(95); // After all elements
    test_assert(result.ts == 90, "Search after all elements in wrapped buffer");
}

// Test 9: Stress test with capacity 1
void test_capacity_one() {
    std::cout << "\n=== Test Capacity One Buffer ===" << std::endl;
    
    StampedRingBuffer<int> buffer(1);
    
    // Add multiple elements, each should replace the previous
    for (int i = 0; i < 5; ++i) {
        buffer.add({(uint64_t)(i * 10), i});
        test_assert(buffer.size() == 1, "Capacity 1 buffer always size 1");
        
        auto result = buffer.binsearch(i * 10);
        test_assert(result.ts == (uint64_t)(i * 10) && result.elem == i, 
                   "Capacity 1 buffer search for current element");
    }
}

// Test 10: Boundary value testing
void test_boundary_values() {
    std::cout << "\n=== Test Boundary Values ===" << std::endl;
    
    StampedRingBuffer<int> buffer(5);
    
    // Use extreme timestamp values
    buffer.add({0, 0});                    // Minimum
    buffer.add({UINT64_MAX / 2, 1});       // Large value
    buffer.add({UINT64_MAX - 1, 2});       // Near maximum
    
    auto result = buffer.binsearch(0);
    test_assert(result.ts == 0, "Search for minimum timestamp");
    
    result = buffer.binsearch(UINT64_MAX);
    test_assert(result.ts == UINT64_MAX - 1, "Search for very large timestamp");
    
    result = buffer.binsearch(UINT64_MAX / 4);
    test_assert(result.ts == 0 || result.ts == UINT64_MAX / 2, "Search between extreme values");
}

int main() {
    std::cout << "Starting comprehensive StampedRingBuffer tests...\n" << std::endl;
    
    try {
        test_basic_functionality();
        test_closest_timestamp_search();
        test_ring_buffer_wraparound();
        test_single_element();
        test_two_elements();
        test_duplicate_timestamps();
        test_large_buffer_random();
        test_complex_wraparound();
        test_capacity_one();
        test_boundary_values();
        
        std::cout << "\n🎉 All tests passed! 🎉" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

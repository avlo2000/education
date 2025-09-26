#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <cassert>

// Include the template implementation directly so definitions are visible
#include "lockfree_array_swmr.cpp"

// Simple test assertion helper
static void test_assert(bool cond, const std::string& name) {
    if (cond) {
        std::cout << "[PASS] " << name << "\n";
    } else {
        std::cout << "[FAIL] " << name << "\n";
        assert(false);
    }
}

// Test 1: Basic single-threaded read-after-write on the same index
static void test_basic_single_thread() {
    std::cout << "\n=== test_basic_single_thread ===\n";
    LockFreeArraySWMR<int> arr(8);

    // Write and immediately read back on same index
    for (size_t i = 0; i < 8; ++i) {
        int v = static_cast<int>(i * 10 + 7);
        arr.write(i, v);
        int r = arr.read(i);
        test_assert(r == v, "Read-after-write matches on index " + std::to_string(i));
    }

    // Re-write same index multiple times; values should update accordingly
    size_t idx = 3;
    for (int r = 1; r <= 5; ++r) {
        arr.write(idx, r);
        int got = arr.read(idx);
        test_assert(got == r, "Sequential updates visible on same index");
    }
}

// Test 2: Single writer, multiple readers on one index — values should be non-decreasing
static void test_swmr_monotonic_single_index() {
    std::cout << "\n=== test_swmr_monotonic_single_index ===\n";
    constexpr size_t N = 1;
    constexpr int iterations = 50000;
    LockFreeArraySWMR<int> arr(N);

    std::atomic<bool> done{false};
    std::atomic<int> max_seen{0};

    // Writer updates index 0 with strictly increasing values
    std::thread writer([&]() {
        for (int v = 1; v <= iterations; ++v) {
            arr.write(0, v);
            if ((v % 5000) == 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        }
        done = true;
    });

    // Multiple readers observe non-decreasing sequence
    const size_t num_readers = 6;
    std::vector<std::thread> readers;
    readers.reserve(num_readers);
    for (size_t t = 0; t < num_readers; ++t) {
        readers.emplace_back([&, t]() {
            int last = 0;
            while (!done.load(std::memory_order_relaxed)) {
                int v = arr.read(0);
                if (v < last) {
                    std::cout << "Monotonicity violation in reader " << t
                              << ": last=" << last << ", now=" << v << "\n";
                    test_assert(false, "Non-decreasing reads for single index");
                }
                last = v;
                int prev_max = max_seen.load();
                while (v > prev_max && !max_seen.compare_exchange_weak(prev_max, v)) {}
            }
            // Final sweep to catch the last value
            for (int i = 0; i < 1000; ++i) {
                int v = arr.read(0);
                if (v < last) {
                    test_assert(false, "Non-decreasing reads in final sweep");
                }
                last = v;
                int prev_max = max_seen.load();
                while (v > prev_max && !max_seen.compare_exchange_weak(prev_max, v)) {}
            }
        });
    }

    writer.join();
    for (auto& th : readers) th.join();

    test_assert(max_seen.load() >= iterations, "Readers eventually observed final value");
}

// Test 3: Struct coherence — ensure readers never see torn updates for a single index
struct Pair {
    uint64_t a;
    uint64_t b;
};

static void test_struct_coherence() {
    std::cout << "\n=== test_struct_coherence ===\n";
    constexpr uint64_t MASK = 0xDEADBEEFCAFEBABEull;
    LockFreeArraySWMR<Pair> arr(2);

    std::atomic<bool> done{false};

    // Writer updates index 1 with coherent Pair values
    std::thread writer([&]() {
        for (uint64_t i = 1; i <= 2000000; ++i) {
            Pair p{i, i ^ MASK};
            arr.write(1, p);
            if ((i % 10000) == 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(20));
            }
        }
        done = true;
    });

    // Readers verify b == a ^ MASK at all times
    const size_t num_readers = 12;
    std::vector<std::thread> readers;
    readers.reserve(num_readers);
    for (size_t t = 0; t < num_readers; ++t) {
        readers.emplace_back([&, t]() {
            while (!done.load(std::memory_order_relaxed)) {
                Pair p = arr.read(1);
                if ((p.a ^ MASK) != p.b) {
                    std::cout << "Torn read in reader " << t << ": a=" << p.a
                              << ", b=" << p.b << "\n";
                    test_assert(false, "Struct fields stay coherent across reads");
                }
            }
            // Final validations after writer finishes
            for (int i = 0; i < 1000; ++i) {
                Pair p = arr.read(1);
                test_assert((p.a ^ MASK) == p.b, "Struct coherence in final sweep");
            }
        });
    }

    writer.join();
    for (auto& th : readers) th.join();
}

int main() {
    std::cout << "Starting LockFreeArraySWMR tests...\n";

    try {
        test_basic_single_thread();
        test_swmr_monotonic_single_index();
        test_struct_coherence();
        std::cout << "\nAll LockFreeArraySWMR tests passed.\n";
    } catch (const std::exception& e) {
        std::cout << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

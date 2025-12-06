#include "stamped_buffer_safe_but_slow.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <mutex>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <set>
#include <memory>
#include <condition_variable>
#include <type_traits>

// Define a trivially copyable struct for testing
struct SomeStruct
{
    uint64_t field1;
    uint64_t field2;
    uint32_t field3;
    uint32_t field4;
};

// Validate that SomeStruct meets requirements
static_assert(std::is_nothrow_copy_assignable_v<SomeStruct>, 
              "SomeStruct must be nothrow copy assignable");
static_assert(std::is_trivially_copy_assignable_v<SomeStruct>, 
              "SomeStruct must be trivially copy assignable");

// Test utility functions
void test_assert(bool condition, const std::string &test_name)
{
    if (condition)
    {
        std::cout << "[PASS] " << test_name << std::endl;
    }
    else
    {
        std::cout << "[FAIL] " << test_name << std::endl;
        assert(false);
    }
}

class MultithreadedTestSuite
{
private:
    std::mutex output_mutex;

    void safe_print(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << message << std::endl;
    }

public:
    // Test 1: Single producer, multiple consumers - validate binary search correctness
    void test_single_producer_multiple_consumers_basic()
    {
        safe_print("\n=== Test Single Producer Multiple Consumers - Binary Search Correctness ===");

        const size_t buffer_capacity = 1000;
        const size_t num_elements = 1000;
        const size_t num_consumers = 4;

        StampedRingBuffer<int> buffer(buffer_capacity);
        std::atomic<bool> producer_done{false};
        std::atomic<size_t> total_searches{0};
        std::atomic<size_t> invalid_results{0};

        // Add initial element to ensure buffer is never empty
        buffer.add({0, 0});

        // Producer thread: adds elements with strictly increasing timestamps
        std::thread producer([&buffer, &producer_done, num_elements]()
                             {
            for (size_t i = 1; i <= num_elements; ++i) {
                uint64_t timestamp = i * 10; // Strictly increasing: 10, 20, 30, ...
                buffer.add({timestamp, static_cast<int>(i)});
                
                // Add small delay to allow consumers to work
                if (i % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            }
            producer_done = true; });

        // Consumer threads: validate binary search correctness
        std::vector<std::thread> consumers;
        for (size_t consumer_id = 0; consumer_id < num_consumers; ++consumer_id)
        {
            consumers.emplace_back([&buffer, &producer_done, &total_searches,
                                    &invalid_results, consumer_id, num_elements]()
                                   {
                std::random_device rd;
                std::mt19937 gen(rd() + consumer_id);
                std::uniform_int_distribution<uint64_t> dis(0, num_elements * 10);
                
                size_t local_searches = 0;
                size_t local_invalid = 0;
                
                while (!producer_done) {
                    uint64_t search_ts = dis(gen);
                    auto result = buffer.binsearch(search_ts);
                    ++local_searches;
                    
                    // Validate that the result is consistent: element should be timestamp/10
                    if (result.ts % 10 != 0 || result.elem != static_cast<int>(result.ts / 10)) {
                        ++local_invalid;
                    }
                    
                    // Also validate that the returned timestamp is reasonable
                    // It should be the closest timestamp to what we searched for
                    // or at least a valid timestamp from the sequence
                    if (result.ts > (num_elements + 1) * 10) {
                        ++local_invalid;
                    }
                    
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
                
                // Continue for a bit after producer is done to catch any remaining issues
                for (int i = 0; i < 100; ++i) {
                    uint64_t search_ts = dis(gen);
                    auto result = buffer.binsearch(search_ts);
                    ++local_searches;
                    
                    if (result.ts % 10 != 0 || result.elem != static_cast<int>(result.ts / 10)) {
                        ++local_invalid;
                    }
                }
                
                total_searches += local_searches;
                invalid_results += local_invalid; });
        }

        producer.join();
        for (auto &consumer : consumers)
        {
            consumer.join();
        }

        // Validate results
        test_assert(total_searches > 0, "Consumers performed searches");
        test_assert(producer_done, "Producer completed");
        test_assert(invalid_results == 0, "No invalid search results (found " +
                                              std::to_string(invalid_results) + " invalid out of " +
                                              std::to_string(total_searches) + " total)");

        safe_print("Binary search correctness test completed with " + std::to_string(total_searches) +
                   " searches, " + std::to_string(invalid_results) + " invalid results");
    }

    // Test 2: Race condition detection - ensure binsearch always returns consistent data
    void test_race_condition_detection()
    {
        safe_print("\n=== Test Race Condition Detection ===");

        const size_t buffer_capacity = 1024; // Small buffer to force frequent wraparound
        const size_t num_elements = 1500000;
        const size_t num_consumers = 12;

        StampedRingBuffer<int> buffer(buffer_capacity);
        std::atomic<bool> producer_done{false};
        std::atomic<size_t> inconsistent_reads{0};
        std::atomic<size_t> total_reads{0};

        // Add initial element
        buffer.add({0, 0});

        // Producer thread: rapidly adds elements
        std::thread producer([&buffer, &producer_done, num_elements]()
                             {
            for (size_t i = 1; i <= num_elements; ++i) {
                uint64_t timestamp = i * 2; // Strictly increasing, even numbers
                buffer.add({timestamp, static_cast<int>(i)});
                
                // Minimal delay to create more race conditions
                if (i % 50 == 0) {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(10));
                }
            }
            producer_done = true; });

        // Consumer threads: aggressively search and validate consistency
        std::vector<std::thread> consumers;
        for (size_t consumer_id = 0; consumer_id < num_consumers; ++consumer_id)
        {
            consumers.emplace_back([&buffer, &producer_done, &inconsistent_reads, &total_reads, consumer_id, num_elements]()
                                   {
                std::random_device rd;
                std::mt19937 gen(rd() + consumer_id);
                std::uniform_int_distribution<uint64_t> dis(0, num_elements * 2);
                
                size_t local_reads = 0;
                size_t local_inconsistent = 0;
                
                while (!producer_done) {
                    uint64_t search_ts = dis(gen);
                    auto result = buffer.binsearch(search_ts);
                    ++local_reads;
                    
                    // Validate that timestamp and element are consistent
                    // Element should be timestamp / 2 (since timestamp = i * 2)
                    if (result.ts % 2 != 0 || result.elem != static_cast<int>(result.ts / 2)) {
                        std::cout << "Inconsistency detected: Buffer size " << buffer.size() << ", searched ts " << search_ts << ", got ts " << result.ts << ", elem " << result.elem << std::endl;
                        // Print buffer min ts and max ts

                        ++local_inconsistent;
                    }
                    
                    // Additional validation: timestamp should not exceed what's been produced
                    if (result.ts > num_elements * 2) {
                        ++local_inconsistent;
                    }
                    auto oldest = buffer.get_oldest();
                    auto newest = buffer.get_newest();
                    if (oldest.ts > newest.ts) {
                        ++local_inconsistent;
                        // assert(oldest.ts - newest.ts == 2);
                        std::cout << "Oldest-newest inversion: diff " << oldest.ts - newest.ts << std::endl;
                    }
                }
                
                // Continue testing for a bit after producer finishes
                for (int i = 0; i < 50; ++i) {
                    uint64_t search_ts = dis(gen);
                    auto result = buffer.binsearch(search_ts);
                    ++local_reads;
                    
                    if (result.ts % 2 != 0 || result.elem != static_cast<int>(result.ts / 2)) {
                        ++local_inconsistent;
                        std::cout << "Inconsistency detected: ts=" << result.ts << ", elem=" << result.elem << std::endl;
                    }

                    // Final rounds also verify oldest <= newest
                    if ((i % 10) == 0) {
                        auto oldest = buffer.get_oldest();
                        auto newest = buffer.get_newest();
                        if (oldest.ts > newest.ts) {
                            ++local_inconsistent;
                            assert(oldest.ts - newest.ts == 2);
                            assert(false);
                            std::cout << "Oldest-newest inversion: diff " << oldest.ts - newest.ts << std::endl;
                        }
                    }
                }
                
                inconsistent_reads += local_inconsistent;
                total_reads += local_reads; });
        }

        producer.join();
        for (auto &consumer : consumers)
        {
            consumer.join();
        }

        test_assert(inconsistent_reads == 0, "No inconsistent reads detected (found " +
                                                 std::to_string(inconsistent_reads) + " inconsistencies out of " +
                                                 std::to_string(total_reads) + " total reads)");
    }

    // Test 3: High frequency producer with timestamp validation
    void test_high_frequency_producer()
    {
        safe_print("\n=== Test High Frequency Producer ===");

        const size_t buffer_capacity = 50;
        const size_t num_elements = 10000;
        const size_t num_consumers = 6;

        StampedRingBuffer<uint64_t> buffer(buffer_capacity);
        std::atomic<bool> producer_done{false};
        std::atomic<uint64_t> max_timestamp_produced{0};
        std::atomic<size_t> timestamp_violations{0};
        std::atomic<size_t> total_validations{0};

        // Add initial element
        buffer.add({1, 1});
        max_timestamp_produced = 1;

        // High frequency producer
        std::thread producer([&buffer, &producer_done, &max_timestamp_produced, num_elements]()
                             {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            for (size_t i = 2; i <= num_elements; ++i) {
                auto now = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time);
                uint64_t timestamp = duration.count() + i; // Ensure strict ordering
                
                buffer.add({timestamp, timestamp}); // Element equals timestamp for easy validation
                max_timestamp_produced = timestamp;
            }
            producer_done = true; });

        // Consumers validate that binary search returns consistent timestamp-element pairs
        std::vector<std::thread> consumers;
        for (size_t consumer_id = 0; consumer_id < num_consumers; ++consumer_id)
        {
            consumers.emplace_back([&buffer, &producer_done, &max_timestamp_produced,
                                    &timestamp_violations, &total_validations, consumer_id]()
                                   {
                std::random_device rd;
                std::mt19937 gen(rd() + consumer_id);
                
                size_t local_violations = 0;
                size_t local_validations = 0;
                
                while (!producer_done) {
                    // Search around the current maximum timestamp
                    uint64_t current_max = max_timestamp_produced.load();
                    if (current_max > 1) {
                        std::uniform_int_distribution<uint64_t> dis(1, current_max);
                        uint64_t search_ts = dis(gen);
                        auto result = buffer.binsearch(search_ts);
                        
                        ++local_validations;
                        
                        // Validate that element equals timestamp
                        if (result.elem != result.ts) {
                            ++local_violations;
                        }
                        
                        // Validate that returned timestamp is not from the future
                        if (result.ts > current_max + 1000) { // Allow some tolerance for timing
                            ++local_violations;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(5));
                }
                
                // Final validation round
                for (int i = 0; i < 100; ++i) {
                    uint64_t final_max = max_timestamp_produced.load();
                    std::uniform_int_distribution<uint64_t> dis(1, final_max);
                    uint64_t search_ts = dis(gen);
                    auto result = buffer.binsearch(search_ts);
                    
                    ++local_validations;
                    if (result.elem != result.ts) {
                        ++local_violations;
                    }
                }
                
                timestamp_violations += local_violations;
                total_validations += local_validations; });
        }

        producer.join();
        for (auto &consumer : consumers)
        {
            consumer.join();
        }

        test_assert(timestamp_violations == 0, "No timestamp-element mismatches (found " +
                                                   std::to_string(timestamp_violations) + " violations out of " +
                                                   std::to_string(total_validations) + " validations)");
        test_assert(max_timestamp_produced > 0, "Observed non-zero timestamps");
    }

    // Test 4.5: XOR integrity data race test using 0xDEADBEEF
    void test_xor_integrity_data_race()
    {
        safe_print("\n=== Test XOR Integrity Under Concurrency (0xDEADBEEF) ===");

        constexpr uint32_t XOR_MASK = 0xDEADBEEF;
        const size_t buffer_capacity = 1024;  // small to increase wraparound
        const size_t num_elements = 1500000; // stress but still completes quickly
        const size_t num_consumers = 12;

        StampedRingBuffer<uint32_t> buffer(buffer_capacity);
        std::atomic<bool> producer_done{false};
        std::atomic<uint64_t> max_ts{0};
        std::atomic<size_t> total_checks{0};
        std::atomic<size_t> xor_violations{0};

        // Seed element: ts=0, elem = 0 ^ MASK
        buffer.add({0, static_cast<uint32_t>(0u ^ XOR_MASK)});

        // Single producer with strictly increasing timestamps
        std::thread producer([&]()
                             {
            for (uint64_t i = 1; i <= num_elements; ++i) {
                uint64_t ts = i; // keep within 32-bit space for XOR with elem
                uint32_t data = static_cast<uint32_t>(ts) ^ XOR_MASK;
                buffer.add({ts, data});
                max_ts.store(ts, std::memory_order_relaxed);

                if ((i % 256ull) == 0ull) {
                    // small pause to shuffle interleavings
                    std::this_thread::sleep_for(std::chrono::nanoseconds(50));
                }
            }
            producer_done.store(true, std::memory_order_release); });

        // Multiple consumers continuously validating XOR relationship
        std::vector<std::thread> consumers;
        consumers.reserve(num_consumers);
        for (size_t id = 0; id < num_consumers; ++id)
        {
            consumers.emplace_back([&, id]()
                                   {
                std::random_device rd;
                std::mt19937 gen(rd() + static_cast<unsigned int>(id));

                size_t local_checks = 0;
                size_t local_violations = 0;

                while (!producer_done.load(std::memory_order_acquire)) {
                    uint64_t cur_max = max_ts.load(std::memory_order_relaxed);
                    if (cur_max == 0) { continue; }

                    std::uniform_int_distribution<uint64_t> dis(0, cur_max);
                    uint64_t search_ts = dis(gen);
                    auto el = buffer.binsearch(search_ts);
                    ++local_checks;

                    // Validate that elem ^ MASK == low32(ts)
                    uint32_t low32_ts = static_cast<uint32_t>(el.ts);
                    if ((el.elem ^ XOR_MASK) != low32_ts) {
                        ++local_violations;
                    }

                    if ((local_checks & 0x3FFu) == 0u) {
                        std::this_thread::sleep_for(std::chrono::microseconds(1));
                    }
                }

                // Final sampling after producer finished
                uint64_t final_max = max_ts.load(std::memory_order_relaxed);
                if (final_max > 0) {
                    std::uniform_int_distribution<uint64_t> dis(0, final_max);
                    for (int i = 0; i < 200; ++i) {
                        uint64_t search_ts = dis(gen);
                        auto el = buffer.binsearch(search_ts);
                        ++local_checks;
                        uint32_t low32_ts = static_cast<uint32_t>(el.ts);
                        if ((el.elem ^ XOR_MASK) != low32_ts) {
                            ++local_violations;
                            assert(false);
                        }
                    }
                }

                total_checks += local_checks;
                xor_violations += local_violations; });
        }

        producer.join();
        for (auto &t : consumers)
            t.join();

        test_assert(total_checks > 0, "Performed XOR validations");
        test_assert(xor_violations == 0, "XOR integrity holds under concurrency (found " +
                                             std::to_string(xor_violations.load()) + " violations out of " +
                                             std::to_string(total_checks.load()) + ")");
    }

    // Test 4: Stress test with wraparound under contention - focus on correctness
    void test_wraparound_stress()
    {
        safe_print("\n=== Test Wraparound Stress - Binary Search Correctness ===");

        const size_t buffer_capacity = 50; // Very small to force frequent wraparound
        const size_t num_elements = 2000;
        const size_t num_consumers = 10;

        StampedRingBuffer<size_t> buffer(buffer_capacity);
        std::atomic<bool> producer_done{false};
        std::atomic<size_t> correct_searches{0};
        std::atomic<size_t> total_search_attempts{0};
        std::atomic<size_t> current_max_element{0};

        // Add initial element
        buffer.add({0, 0});

        // Producer rapidly fills small buffer
        std::thread producer([&buffer, &producer_done, &current_max_element, num_elements]()
                             {
            for (size_t i = 1; i <= num_elements; ++i) {
                buffer.add({i, i});
                current_max_element = i;
                
                // Occasional micro-sleep to create timing variations
                if (i % 10 == 0) {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(50));
                }
            }
            producer_done = true; });

        // Many consumers competing for searches, validating correctness
        std::vector<std::thread> consumers;
        for (size_t consumer_id = 0; consumer_id < num_consumers; ++consumer_id)
        {
            consumers.emplace_back([&buffer, &producer_done, &correct_searches,
                                    &total_search_attempts, &current_max_element, consumer_id, num_elements]()
                                   {
                std::random_device rd;
                std::mt19937 gen(rd() + consumer_id);
                
                size_t local_attempts = 0;
                size_t local_correct = 0;
                
                while (!producer_done) {
                    size_t max_so_far = current_max_element.load();
                    if (max_so_far > 0) {
                        std::uniform_int_distribution<uint64_t> dis(0, max_so_far);
                        uint64_t search_ts = dis(gen);
                        auto result = buffer.binsearch(search_ts);
                        
                        ++local_attempts;
                        
                        // Validate result: timestamp should equal element and be within valid range
                        if (result.ts == result.elem && result.ts <= max_so_far + buffer_capacity) {
                            ++local_correct;
                        } else {
                            std::cout << "Wraparound inconsistency: searched " << search_ts 
                                      << ", got ts=" << result.ts << ", elem=" << result.elem << std::endl;
                        }
                    }
                }
                
                // Final round of testing after producer finishes
                for (int i = 0; i < 100; ++i) {
                    std::uniform_int_distribution<uint64_t> dis(0, num_elements);
                    uint64_t search_ts = dis(gen);
                    auto result = buffer.binsearch(search_ts);
                    
                    ++local_attempts;
                    
                    // At this point, buffer should contain the last buffer_capacity elements
                    size_t expected_min = num_elements > buffer_capacity ? num_elements - buffer_capacity + 1 : 0;
                    if (result.ts == result.elem && result.ts >= expected_min && result.ts <= num_elements) {
                        ++local_correct;
                    } else {
                        std::cout << "Wraparound inconsistency: searched " << search_ts 
                                  << ", got ts=" << result.ts << ", elem=" << result.elem << std::endl;
                    }
                }
                
                total_search_attempts += local_attempts;
                correct_searches += local_correct; });
        }

        producer.join();
        for (auto &consumer : consumers)
        {
            consumer.join();
        }

        test_assert(correct_searches > 0, "Some searches were correct");
        test_assert(total_search_attempts > 0, "Search attempts were made");

        double correctness_rate = static_cast<double>(correct_searches) / total_search_attempts;
        safe_print("Wraparound stress test: " + std::to_string(correct_searches) + "/" +
                   std::to_string(total_search_attempts) + " searches correct (" +
                   std::to_string(correctness_rate * 100) + "%)");

        // We expect very high correctness rate for a working lock-free structure
        test_assert(correctness_rate > 0.95, "High correctness rate under stress (expected >95%, got " +
                                                 std::to_string(correctness_rate * 100) + "%)");
    }

    // Test with SomeStruct - verify type traits and size
    void test_stamped_buffer_with_struct()
    {
        safe_print("\n=== Test StampedRingBuffer<SomeStruct> - Type Traits and Size ===");

        // Verify size of StampedRingBuffer<SomeStruct> instance
        StampedRingBuffer<SomeStruct> buffer(10);
        size_t buffer_size = sizeof(buffer);
        
        safe_print("Size of StampedRingBuffer<SomeStruct>: " + std::to_string(buffer_size) + " bytes");
        // Functional test: add and retrieve elements
        const size_t num_elements = 100;
        const size_t num_consumers = 4;

        std::atomic<bool> producer_done{false};
        std::atomic<size_t> validation_failures{0};

        // Producer: add elements with SomeStruct
        std::thread producer([&buffer, &producer_done, num_elements]()
                             {
            for (size_t i = 1; i <= num_elements; ++i) {
                uint64_t timestamp = i * 100;
                SomeStruct data{timestamp, i, static_cast<uint32_t>(i * 2), static_cast<uint32_t>(i * 3)};
                buffer.add({timestamp, data});
                
                if (i % 20 == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            }
            producer_done = true; });

        // Consumers: validate data integrity
        std::vector<std::thread> consumers;
        for (size_t consumer_id = 0; consumer_id < num_consumers; ++consumer_id)
        {
            consumers.emplace_back([&buffer, &producer_done, &validation_failures, consumer_id, num_elements]()
                                   {
                std::random_device rd;
                std::mt19937 gen(rd() + consumer_id);
                std::uniform_int_distribution<uint64_t> dis(100, num_elements * 100);
                
                while (!producer_done) {
                    uint64_t search_ts = dis(gen);
                    auto result = buffer.binsearch(search_ts);
                    
                    // Validate struct fields are consistent with timestamp
                    uint64_t expected_idx = result.ts / 100;
                    if (result.elem.field1 != result.ts ||
                        result.elem.field2 != expected_idx ||
                        result.elem.field3 != static_cast<uint32_t>(expected_idx * 2) ||
                        result.elem.field4 != static_cast<uint32_t>(expected_idx * 3)) {
                        validation_failures++;
                    }
                    
                    std::this_thread::sleep_for(std::chrono::microseconds(5));
                }
                
                // Final validation after producer completes
                for (int i = 0; i < 50; ++i) {
                    uint64_t search_ts = dis(gen);
                    auto result = buffer.binsearch(search_ts);
                    
                    uint64_t expected_idx = result.ts / 100;
                    if (result.elem.field1 != result.ts ||
                        result.elem.field2 != expected_idx ||
                        result.elem.field3 != static_cast<uint32_t>(expected_idx * 2) ||
                        result.elem.field4 != static_cast<uint32_t>(expected_idx * 3)) {
                        validation_failures++;
                    }
                } });
        }

        producer.join();
        for (auto &consumer : consumers)
        {
            consumer.join();
        }

        test_assert(validation_failures == 0, 
                    "No validation failures with SomeStruct (found " +
                    std::to_string(validation_failures) + " failures)");

        safe_print("StampedRingBuffer<SomeStruct> test completed successfully");
    }

    void run_all_tests()
    {
        safe_print("Starting comprehensive multi-threaded StampedRingBuffer tests...\n");

        try
        {
            test_single_producer_multiple_consumers_basic();
            test_xor_integrity_data_race();
            test_race_condition_detection();
            test_high_frequency_producer();
            test_wraparound_stress();
            test_stamped_buffer_with_struct();

            safe_print("\nAll multi-threaded tests passed!");
        }
        catch (const std::exception &e)
        {
            safe_print("\nMulti-threaded test failed with exception: " + std::string(e.what()));
            throw;
        }
    }
};

int main()
{
    MultithreadedTestSuite test_suite;
    test_suite.run_all_tests();
    return 0;
}

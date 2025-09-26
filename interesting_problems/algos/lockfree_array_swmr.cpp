#include <vector>
#include <atomic>
#include <memory>

template <typename T>
class LockFreeArraySWMR {
public:
    explicit LockFreeArraySWMR(size_t size) {
        // Initialize two buffers; capacity fixed to avoid reallocations.
        buffers_[0] = std::make_unique<std::vector<T>>(size);
        buffers_[1] = std::make_unique<std::vector<T>>(size);
        epoch_.store(0, std::memory_order_relaxed); // even number indicates a stable snapshot
    }

    // Single-writer: write a value at index.
    // Uses a double-buffer seqlock scheme. Writer updates the inactive buffer then flips epoch.
    void write(size_t index, const T& value) {
        uint8_t e = epoch_.load(std::memory_order_relaxed);
        int active = static_cast<int>(e & 1ULL);
        int inactive = active ^ 1;

        // Write to inactive buffer only.
        (*buffers_[inactive])[index] = value;

        // Publish new snapshot: increment epoch (flips buffer index).
        // Release ensures the write to the inactive buffer is visible before readers observe new epoch.
        epoch_.store(e + 1, std::memory_order_release);
    }

    // Reader: obtains a consistent snapshot by retrying if a write occurs during the read.
    T read(size_t index) const {
        for (;;) {
            uint8_t e1 = epoch_.load(std::memory_order_acquire);
            int buf = static_cast<int>(e1 & 1);
            // Read value from the chosen snapshot
            T val = (*buffers_[buf])[index];
            // Recheck that no writer published a new snapshot during the read
            uint8_t e2 = epoch_.load(std::memory_order_acquire);
            if (e1 == e2) {
                return val;
            }
            // Writer raced; retry to avoid torn/composite reads.
        }
    }

private:
    std::unique_ptr<std::vector<T>> buffers_[2];
    // Monotonic epoch; parity selects active buffer. Readers ensure stability by comparing before/after.
    mutable std::atomic<uint8_t> epoch_;
};
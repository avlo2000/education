// Single-producer, multi-consumer stamped ring buffer with per-slot seqlock.
// Notes:
// - One writer thread max. Many readers allowed concurrently.
// - Writer publishes to a slot using a per-slot sequence (odd = writing, even = stable).
// - Readers use seqlock-style double-read to avoid data races/tearing.
// - Timestamps in the logical window [start_, start_+size_-1] are monotonically increasing.

#ifndef LOCKFREE_BUFFER_HPP
#define LOCKFREE_BUFFER_HPP
#include <atomic>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>

template <class T>
struct StampedElement
{
    StampedElement() : ts(0), elem() {}
    StampedElement(uint64_t ts, T elem) : ts(ts), elem(elem) {}
    uint64_t _pad[8]; // padding to avoid false sharing. May be adjusted for better performance
    uint64_t ts;
    T elem;
};

template <class T>
class StampedRingBuffer
{
    using ElType = StampedElement<T>;
    struct Slot {
        // Seqlock value: even => stable, odd => being written.
        std::atomic<uint64_t> seq{0};
        uint64_t ts{0};
        T elem{};
        // Light padding to reduce false sharing across slots; tune if needed.
        uint64_t _pad[3] = {0,0,0};
    };

public:
    StampedRingBuffer(size_t cap) : cap_(cap)
    {
        assert(cap_ > 0 && "capacity must be > 0");
        buffer_ = new Slot[cap_];
    }

    ~StampedRingBuffer()
    {
        delete[] buffer_;
    }

    // supports only single producer
    void add(const ElType &el)
    {
        // Snapshot current window
        size_t start = start_.load(std::memory_order_relaxed);
        size_t size = size_.load(std::memory_order_relaxed);

        // Compute write index at logical end of the window
        size_t idx = (start + size) % cap_;

        // Publish payload with per-slot seqlock (odd -> write, even -> stable)
        Slot &slot = buffer_[idx];
        slot.seq.fetch_add(1, std::memory_order_release); // odd => writing
        slot.ts = el.ts;
        slot.elem = el.elem;
        slot.seq.fetch_add(1, std::memory_order_release); // even => stable

        // Advance window (overwrite oldest when full)
        if (size == cap_) {
            start = (start + 1) % cap_;
        } else {
            ++size;
        }
        // Publish the updated window. A single release on size_ is enough, but keep both consistent.
        start_.store(start, std::memory_order_release);
        size_.store(size, std::memory_order_release);
    }

    // Binary search over a stable snapshot [start, start+size-1].
    // Returns the element closest on the right of ts (lower_bound), with a simple neighbor check
    // to pick the better of prev vs current.
    // If the window changes during search, the function transparently retries.
    ElType binsearch(uint64_t qts) const
    {
        for (int attempt = 0; attempt < 8; ++attempt) {
            size_t start = start_.load(std::memory_order_acquire);
            size_t size = size_.load(std::memory_order_acquire);
            if (size == 0) return ElType();

            auto map = [&](size_t rank) { return (start + rank) % cap_; };

            // Fast-path boundaries
            ElType firstEl, lastEl;
            if (!read_slot(map(0), firstEl)) continue; // retry snapshot
            if (size == 1) return firstEl;
            if (!read_slot(map(size - 1), lastEl)) continue;
            if (qts <= firstEl.ts) return firstEl;
            if (qts >= lastEl.ts) return lastEl;

            // lower_bound in [0, size-1]
            size_t lo = 0, hi = size - 1;
            while (lo < hi) {
                size_t mid = lo + ((hi - lo) >> 1);
                ElType midEl;
                if (!read_slot(map(mid), midEl)) { lo = 0; hi = size - 1; continue; }
                if (midEl.ts < qts) lo = mid + 1; else hi = mid;
            }

            // lo is first index with ts >= qts
            size_t curRank = lo;
            size_t prevRank = (curRank == 0) ? 0 : (curRank - 1);
            ElType curEl, prevEl;
            if (!read_slot(map(curRank), curEl)) continue;
            if (!read_slot(map(prevRank), prevEl)) continue;

            // Choose closer by timestamp; prefer cur when equal.
            if (qts - prevEl.ts <= curEl.ts - qts) return prevEl;
            return curEl;
        }
        // Fallback: try to return a consistent oldest element to avoid UB.
        return get_oldest();
    }

    ElType get_oldest() const
    {
        size_t s = start_.load(std::memory_order_acquire);
        ElType el;
        if (read_slot(s, el)) return el;
        // Try a couple of neighbors if start moved during read
        size_t s2 = start_.load(std::memory_order_acquire);
        if (read_slot(s2, el)) return el;
        // As a last resort, return a default element
        return ElType();
    }

    // Not thread safe
    void print(bool nord) const
    {
        if (nord)
        {
            size_t s = size_.load(std::memory_order_acquire);
            size_t st = start_.load(std::memory_order_acquire);
            for (size_t i = 0; i < s; ++i)
                if (i != st) {
                    ElType el; if (read_slot(i, el)) std::cout << el.ts << " "; else std::cout << "? ";
                } else {
                    ElType el; if (read_slot(i, el)) std::cout << "[" << el.ts << "] "; else std::cout << "[?] ";
                }
            std::cout << std::endl;
            return;
        }
        size_t s = size_.load(std::memory_order_acquire);
        size_t st = start_.load(std::memory_order_acquire);
        for (size_t i = 0; i < s; ++i) {
            size_t idx = (st + i) % cap_;
            ElType el; bool ok = read_slot(idx, el);
            if (idx != st)
                std::cout << (ok ? std::to_string(el.ts) : std::string("?")) << " ";
            else
                std::cout << "[" << (ok ? std::to_string(el.ts) : std::string("?")) << "] ";
        }
        std::cout << std::endl;
    }

    bool empty() const { return size_ == 0ull; }

    size_t size() const { return size_; }

private:
    // Seqlock read of a slot: returns true if a consistent snapshot was read into out.
    inline bool read_slot(size_t idx, ElType &out) const {
        // Limit spinning to avoid livelock under heavy write pressure.
        for (int i = 0; i < 16; ++i) {
            uint64_t s1 = buffer_[idx].seq.load(std::memory_order_acquire);
            if (s1 & 1u) continue; // being written
            uint64_t ts = buffer_[idx].ts;
            T elem = buffer_[idx].elem;
            uint64_t s2 = buffer_[idx].seq.load(std::memory_order_acquire);
            if (s1 == s2 && !(s2 & 1u)) {
                out.ts = ts;
                out.elem = elem;
                return true;
            }
        }
        return false;
    }
private:
    std::atomic<size_t> start_ = 0; // index of the oldest element (logical start)
    Slot* buffer_;
    size_t cap_;
    std::atomic<size_t> size_ = 0;  // number of valid elements in the window
};

#endif

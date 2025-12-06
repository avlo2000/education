#ifndef LOCKFREE_BUFFER_HPP
#define LOCKFREE_BUFFER_HPP
#include <atomic>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include "seq_lock.h"

template <class T>
struct StampedElement
{
    StampedElement() : ts(0), elem() {}
    StampedElement(uint64_t ts, T elem) : ts(ts), elem(elem) {}
    uint64_t ts;
    T elem;
};

template <class T>
class StampedRingBuffer
{
    using ElType = StampedElement<T>;

public:
    StampedRingBuffer(size_t cap) : cap_(cap)
    {
        buffer_ = new rigtorp::Seqlock<ElType>[cap_];
    }

    ~StampedRingBuffer()
    {
        delete[] buffer_;
    }

    // supports only single producer
    void add(const ElType &el)
    {
        auto [start, size] = idx_.load(std::memory_order_relaxed);
        if (size == cap_)
            start = (start + 1) % cap_;
        else
            size++;
        idx_.store({start, size}, std::memory_order_release);
        size_t end = (start + size - 1ull) % cap_;
        buffer_[end].store(el);
    }

    ElType get_oldest() const
    {
        size_t start = idx_.load(std::memory_order_acquire).start;
        return get(start, start);
    }

    ElType get_newest() const
    {
        auto [start, size] = idx_.load(std::memory_order_acquire);
        size_t end = (start + size - 1ull) % cap_;
        return get(end, end);
    }

    ElType binsearch(uint64_t ts) const
    {
        auto [start, len] = idx_.load(std::memory_order_acquire);
        return binsearch(ts, start, len);
    }

    // Not thread safe
    void print(bool nord) const
    {
        if (nord)
        {
            for (size_t i = 0; i < idx_.load(std::memory_order_acquire).size; ++i)
                if (i != idx_.load(std::memory_order_acquire).start)
                    std::cout << buffer_[i].load().ts << " ";
                else
                    std::cout << "[" << buffer_[i].load().ts << "] ";
            std::cout << std::endl;
            return;
        }
        for (size_t i = 0; i < idx_.load(std::memory_order_acquire).size; ++i)
            if (i != idx_.load(std::memory_order_acquire).start)
                std::cout << buffer_[(idx_.load(std::memory_order_acquire).start + i) % cap_].load().ts << " ";
            else
                std::cout << "[" << buffer_[(idx_.load(std::memory_order_acquire).start + i) % cap_].load().ts << "] ";
        std::cout << std::endl;
    }

    bool empty() const { return idx_.load(std::memory_order_acquire).size == 0ull; }

    size_t size() const { return idx_.load(std::memory_order_acquire).size; }

private:
    // if size == 0 causes UB
    ElType binsearch(uint64_t ts, size_t first, size_t len) const
    {
        size_t first_rel = 0;
        while (len > 0ull)
        {
            size_t half = len >> 1;
            size_t mid_rel = first_rel + half;
            size_t mid = (first + mid_rel) % cap_;
            // if mid was changed during bisect in case if should exit
            // it doesn't gurantee optimal result, but prevents race condition
            if (get(first, mid).ts < ts)
            {
                first_rel = mid_rel + 1;
                len = len - half - 1ull;
            }
            else
            {
                len = half;
            }
        }
        size_t size = idx_.load(std::memory_order_acquire).size;
        if (first_rel >= size)
        {
            first_rel = size - 1;
        }
        size_t idx = (first + first_rel) % cap_;
        ElType el_first = get(first, idx);
        return el_first;
    }

    inline ElType get(size_t prev_start, size_t &idx) const
    {
        ElType el;
        while (!try_get(el, prev_start, idx))
        {
            ++idx;
            if (idx == idx_.load(std::memory_order_acquire).size) idx = 0;
            idx %= cap_;
        }
        return el;
    }

    inline bool try_get(ElType &el, size_t prev_start, size_t idx) const
    {
        el = buffer_[idx].load();
        return !was_changed(idx, prev_start);
    }

    inline bool was_changed(size_t i, size_t prev_start) const
    {
        size_t start = idx_.load(std::memory_order_acquire).start;
        if (start >= prev_start)
            return (i >= prev_start) && (i < start);
        return (i >= prev_start) || (i < start);
    }

private:
    struct Indexing
    {
        uint32_t start = 0;
        uint32_t size = 0;
    };
    std::atomic<Indexing> idx_ = {};
    rigtorp::Seqlock<ElType> *buffer_;
    size_t cap_;
};

#endif

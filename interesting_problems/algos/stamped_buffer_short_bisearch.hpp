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
    uint64_t _pad[2]; // padding to avoid false sharing. May be adjusted for better performance
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
        buffer_ = new ElType[cap_];
    }

    ~StampedRingBuffer()
    {
        delete[] buffer_;
    }

    // supports only single producer
    void add(const ElType &el)
    {
        auto [start, size] = idx_.load(std::memory_order_relaxed);
        if (size == cap_) start = (start + 1) % cap_;
        else size++;
        idx_.store({start, size}, std::memory_order_release);
        size_t end = (start + size - 1ull) % cap_;
        buffer_[end] = el;
    }

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
            // it doesn't gurantee optimal result, but prevents Race condition
            if (buffer_[mid].ts < ts)
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
        if (first_rel >= size)  {
            first_rel = size - 1;
        }
        size_t idx = (first + first_rel) % cap_;
        ElType el_first = get(first, idx);
        return el_first;
    }

    ElType get_oldest() const
    {
        size_t start = idx_.load(std::memory_order_acquire).start;
        return buffer_[start];
    }

    ElType get_newest() const
    {
        auto [start, size] = idx_.load(std::memory_order_acquire);
        size_t end = (start + size - 1ull) % cap_;
        return buffer_[end];
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
                    std::cout << buffer_[i].ts << " ";
                else
                    std::cout << "[" << buffer_[i].ts << "] ";
            std::cout << std::endl;
            return;
        }
        for (size_t i = 0; i < idx_.load(std::memory_order_acquire).size; ++i)
            if (i != idx_.load(std::memory_order_acquire).start)
                std::cout << buffer_[(idx_.load(std::memory_order_acquire).start + i) % cap_].ts << " ";
            else
                std::cout << "[" << buffer_[(idx_.load(std::memory_order_acquire).start + i) % cap_].ts << "] ";
        std::cout << std::endl;
    }

    bool empty() const { return idx_.load(std::memory_order_acquire).size == 0ull; }

    size_t size() const { return idx_.load(std::memory_order_acquire).size; }

private:
    inline void get_pair(size_t prev_start, size_t idx, ElType& el1, ElType& el2) const
    {
        while (!try_get_pair(el1, el2, prev_start, idx)) {++idx; idx %= cap_; }
    }

    inline bool try_get_pair(ElType& el1, ElType& el2, size_t prev_start, size_t idx) const
    {
        size_t idx2 = (idx + 1ull) % idx_.load(std::memory_order_acquire).size;
        el1 = buffer_[idx];
        el2 = buffer_[idx2];
        return !was_changed(idx, prev_start);
    }

    inline ElType get(size_t prev_start, size_t& idx) const
    {
        ElType el;
        while (!try_get(el, prev_start, idx)) {++idx; idx %= cap_; }
        return el;
    }

    inline bool try_get(ElType& el, size_t prev_start, size_t idx) const
    {
        el = buffer_[idx];
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
    std::atomic<uint64_t> *timestamps_;

    ElType* buffer_;
    size_t cap_;
};

#endif

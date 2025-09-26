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
        size_t start = idx_.load(std::memory_order_relaxed);
        size_t size = size_.load(std::memory_order_relaxed);
        if (size == cap_) start = (start + 1) % cap_;
        else size++;
        size_.store(size, std::memory_order_release);
        idx_.store(start, std::memory_order_release);
        size_t end = (start + size - 1ull) % cap_;
        buffer_[end] = el;
        end_.store(end, std::memory_order_release);
    }

    // if size == 0 causes UB
    ElType binsearch(uint64_t ts, size_t first, size_t len) const
    {
        size_t start = first;
        size_t size = len;
        while (len > 2ull)
        {
            size_t half = len >> 1;
            size_t mid = (first + half) % cap_;
            // if mid was changed during bisect in case if should exit
            // it doesn't gurantee optimal result, but prevents Race condition
            if (get(start, mid).ts < ts)
            {
                first = (mid + 1) % cap_;
                len = len - half - 1ull;
            }
            else
            {
                len = half;
            }
        } ;
        // ElType el_first = get(start, first);
        // size_t second_idx = (first + 1) % size_.load(std::memory_order_acquire);
        // ElType el_second = get(start, second_idx);
        ElType el_first, el_second;
        get_pair(start, first, el_first, el_second);
        if (el_first.ts < el_second.ts && ts > el_first.ts)
            return el_second;
        return el_first;
    }

    ElType get_oldest() const
    {
        size_t start = idx_.load(std::memory_order_acquire);
        return buffer_[start];
    }

    ElType get_newest() const
    {
        size_t end = end_.load(std::memory_order_acquire);
        return buffer_[end_];
    }

    ElType binsearch(uint64_t ts) const
    {
        size_t start = idx_;
        size_t len = size_;
        return binsearch(ts, start, len);
    }

    // Not thread safe
    void print(bool nord) const
    {
        if (nord)
        {
            for (size_t i = 0; i < size_; ++i)
                if (i != idx_)
                    std::cout << buffer_[i].ts << " ";
                else
                    std::cout << "[" << buffer_[i].ts << "] ";
            std::cout << std::endl;
            return;
        }
        for (size_t i = 0; i < size_; ++i)
            if (i != idx_)
                std::cout << buffer_[(idx_ + i) % cap_].ts << " ";
            else
                std::cout << "[" << buffer_[(idx_ + i) % cap_].ts << "] ";
        std::cout << std::endl;
    }

    bool empty() const { return size_ == 0ull; }

    size_t size() const { return size_; }

private:
    inline void get_pair(size_t prev_start, size_t idx, ElType& el1, ElType& el2) const
    {
        while (!try_get_pair(el1, el2, prev_start, idx)) {++idx; idx %= cap_; }
    }

    inline bool try_get_pair(ElType& el1, ElType& el2, size_t prev_start, size_t idx) const
    {
        size_t idx2 = (idx + 1ull) % size_.load(std::memory_order_acquire);
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
        size_t start = idx_.load(std::memory_order_acquire);
        if (start >= prev_start)
            return (i >= prev_start) && (i < start);
        return (i >= prev_start) || (i < start);
    }
private:
    std::atomic<size_t> idx_ = 0;
    std::atomic<size_t> end_ = 0;
    ElType* buffer_;
    size_t cap_;
    std::atomic<size_t> size_ = 0;
};

#endif

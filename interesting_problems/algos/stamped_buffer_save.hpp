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
        size_t start = start_.load(std::memory_order_relaxed);
        size_t size = size_.load(std::memory_order_relaxed);
        if (size == cap_) start = (start + 1) % cap_;
        else size++;
        size_.store(size, std::memory_order_release);
        start_.store(start, std::memory_order_release);
        buffer_[(start + size) % cap_] = el;
    }

    // if size == 0 causes UB
    size_t binsearch(uint64_t ts, size_t first, size_t len) const
    {
        size_t start = first;
        size_t size = len;
        while (len > 2ull)
        {
            size_t half = len >> 1;
            size_t mid = (first + half) % cap_;
            // if mid was changed during bisect in case if should exit
            // it doesn't gurantee optimal result, but prevents Race condition
            if (was_changed(mid, start))
                return mid;
            if (buffer_[mid].ts < ts)
            {
                first = mid;
                ++first;
                first %= cap_;
                len = len - half - 1ull;
            }
            else
            {
                len = half;
            }
        }
        size_t second = (first + 1) % size;
        size_t ts0 = buffer_[first].ts;
        size_t ts1 = buffer_[second].ts;
        size_t choice = first;
        if (ts0 < ts1 && ts > ts0)
            choice = second;
        return choice;
    }

    ElType get_oldest() const
    {
        return buffer_[start_];
    }

    ElType binsearch(uint64_t ts) const
    {
        size_t start = start_;
        size_t first = start, len = size_.load(std::memory_order_relaxed);
        size_t choice = binsearch(ts, first, len);
        return get(start, choice);
    }

    // Not thread safe
    void print(bool nord) const
    {
        if (nord)
        {
            for (size_t i = 0; i < size_; ++i)
                if (i != start_)
                    std::cout << buffer_[i].ts << " ";
                else
                    std::cout << "[" << buffer_[i].ts << "] ";
            std::cout << std::endl;
            return;
        }
        for (size_t i = 0; i < size_; ++i)
            if (i != start_)
                std::cout << buffer_[(start_ + i) % cap_].ts << " ";
            else
                std::cout << "[" << buffer_[(start_ + i) % cap_].ts << "] ";
        std::cout << std::endl;
    }

    bool empty() const { return size_ == 0ull; }

    size_t size() const { return size_; }

private:
    inline ElType get(size_t prev_start, size_t idx) const
    {
        ElType el;
        while (!try_get(el, prev_start, idx)) {++idx; idx %= cap_; }
        return el;
    }

    inline bool try_get(ElType& el, size_t prev_start, size_t idx) const
    {
        el = buffer_[idx];
        if(idx == start_.load(std::memory_order_acquire))
            return false;
        return !was_changed(idx, prev_start);
    }

    inline bool was_changed(size_t i, size_t prev_start) const
    {
        size_t start = start_.load(std::memory_order_acquire);
        if (start >= prev_start)
            return (i >= prev_start) && (i < start);
        return (i >= prev_start) || (i < start);
    }
private:
    std::atomic<size_t> start_ = 0;
    ElType* buffer_;
    size_t cap_;
    std::atomic<size_t> size_ = 0;
};

#endif

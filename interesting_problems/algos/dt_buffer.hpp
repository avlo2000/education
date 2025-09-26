#ifndef LOCKFREE_BUFFER_HPP
#define LOCKFREE_BUFFER_HPP
#include <atomic>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>

template <class T>
struct Element
{
    DtElement() : ts(0), elem() {}
    DtElement(uint64_t ts, T elem) : ts(ts), elem(elem) {}
    uint64_t _pad[2]; // padding to avoid false sharing. May be adjusted for better performance
    uint64_t ts;
    T elem;
};

template <class T>
class DtRingBuffer
{
    using ElType = DtElement<T>;

public:
    DtRingBuffer(size_t cap, uint32_t period) : cap_(cap)
    {
        buffer_ = new ElType[cap_];
        start_.store(0, std::memory_order_relaxed);
        size_.store(0, std::memory_order_relaxed);
        period_ = period;
    }

    ~DtRingBuffer()
    {
        delete[] buffer_;
    }

    // supports only single producer
    void add(const ElType &el)
    {
        size_t start = start_.load(std::memory_order_acquire);
        size_t size = size_.load(std::memory_order_acquire);
        if (size == cap_) {
            start = (start + 1) % cap_;
            start_.store(start, std::memory_order_release);
        }
        else {
            size++;
            size_.store(size, std::memory_order_release);
        }

        size_t end = (start + size - 1ull) % cap_;
        buffer_[end] = el;
    }

    void get(uint64_t ts)
    {
        
    }

    // Not thread safe
    void print(bool nord) const
    {
        size_t start = start_.load(std::memory_order_acquire);
        size_t size = size_.load(std::memory_order_acquire);
        if (nord)
        {

            for (size_t i = 0; i < size; ++i)
                if (i != start)
                    std::cout << buffer_[i].ts << " ";
                else
                    std::cout << "[" << buffer_[i].ts << "] ";
            std::cout << std::endl;
            return;
        }
        for (size_t i = 0; i < size; ++i)
            if (i != start)
                std::cout << buffer_[(start + i) % cap_].ts << " ";
            else
                std::cout << "[" << buffer_[(start + i) % cap_].ts << "] ";
        std::cout << std::endl;
    }

    bool empty() const { return size_.load(std::memory_order_acquire) == 0ull; }

    size_t size() const { return size_.load(std::memory_order_acquire); }

private:
    inline bool was_changed(size_t i, size_t prev_start) const
    {
        size_t start = idx_.load(std::memory_order_acquire).start;
        if (start >= prev_start)
            return (i >= prev_start) && (i < start);
        return (i >= prev_start) || (i < start);
    }
private:
    std::atomic<size_t> start_;
    std::atomic<size_t> size_;
    uint32_t period_;
    ElType *buffer_;
    size_t cap_;
};

#endif

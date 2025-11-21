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
        end_.store(0, std::memory_order_relaxed);
        period_ = period;
    }

    ~DtRingBuffer()
    {
        delete[] buffer_;
    }

    size_t incr(size_t idx) {
        return (idx + 1) % cap_;
    }

    // supports only single producer
    void add(const ElType &el)
    {
        size_t start = start_.load(std::memory_order_acquire);
        size_t end = end_.load(std::memory_order_acquire);
        if (end - start >= cap_) { // Full
            start = incr(start);
        }
        buffer_[end] = el;
        end = incr(end);
        start_.store
    }

    void get(uint64_t ts)
    {
        
    }

    // Not thread safe
    void print(bool nord) const
    {
        size_t start = start_.load(std::memory_order_acquire);
        size_t size = end_.load(std::memory_order_acquire);
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

    bool empty() const { return size() == 0ull; }

    size_t size() const { return end_.load(std::memory_order_acquire) - start_.load(std::memory_order_acquire); }

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
    std::atomic<size_t> end_;
    uint32_t period_;
    ElType *buffer_;
    size_t cap_;
};

#endif

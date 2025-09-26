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
        epoch_ = new std::atomic<uint8_t>[cap_];
        start_.store(0);
        size_.store(0);
        for (size_t i = 0; i < cap_; ++i)
            epoch_[i].store(0, std::memory_order_relaxed);
    }

    ~StampedRingBuffer()
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
        epoch_[end].fetch_add(1, std::memory_order_release); // element end is in new epoch
    }

    // if size == 0 causes UB
    ElType binsearch(uint64_t ts, size_t first, size_t len) const
    {
        uint8_t epoch = epoch_[first + 1].load(std::memory_order_acquire);
        size_t first_rel = 0;
        while (len > 0ull)
        {
            size_t half = len >> 1;
            size_t mid_rel = first_rel + half;
            size_t mid = (first + mid_rel) % cap_;

            if (atomic_get(epoch, mid).ts < ts)
            {
                first_rel = mid_rel + 1;
                len = len - half - 1ull;
            }
            else
            {
                len = half;
            }
        }
        size_t size = size_.load(std::memory_order_acquire);
        if (first_rel >= size)
        {
            first_rel = size - 1;
        }
        size_t idx = (first + first_rel) % cap_;
        ElType el_first = atomic_get(epoch, idx);
        return el_first;
    }

    ElType binsearch(uint64_t ts) const
    {
        size_t start = start_.load(std::memory_order_acquire);
        size_t size = size_.load(std::memory_order_acquire);
        return binsearch(ts, start, size);
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
    inline ElType atomic_get_oldest(uint8_t epoch) const {
        size_t start = start_.load(std::memory_order_acquire);
        return atomic_get(start);
    }

    inline ElType atomic_get(uint8_t epoch, size_t &idx) const {
        size_t size = size_.load(std::memory_order_acquire);
        ElType el;
        el = buffer_[idx];
        size_t roll_over_sz = size;
        while(epoch_[idx].load(std::memory_order_acquire) - epoch == 1) {
            idx = (idx + 1) % size;
            if(roll_over_sz-- == 0) { epoch++; idx = start_.load(std::memory_order_acquire); };
            el = buffer_[idx];
        }
        return el;
    }

private:
    std::atomic<size_t> start_;
    std::atomic<size_t> size_;
    std::atomic<uint8_t> *epoch_{};
    ElType *buffer_;
    size_t cap_;
};

#endif

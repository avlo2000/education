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

    void add(const ElType &el)
    {
        if (size_ == cap_)
            start_ = (start_ + 1) % cap_;
        else
            size_++;
        size_t end = (start_ + size_ - 1ull) % cap_;
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
        if (first_rel >= size_)
        {
            first_rel = size_ - 1;
        }
        size_t idx = (first + first_rel) % cap_;
        ElType el_first = buffer_[idx];
        return el_first;
    }

    ElType get_oldest() const
    {
        return buffer_[start_];
    }

    ElType get_newest() const
    {
        size_t end = (start_ + size_ - 1ull) % cap_;
        return buffer_[end];
    }

    ElType binsearch(uint64_t ts) const
    {
        return binsearch(ts, start_, size_);
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
    uint32_t start_ = 0;
    uint32_t size_ = 0;
    ElType *buffer_;
    size_t cap_;
};

#endif

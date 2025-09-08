#ifndef LOCKFREE_BUFFER_HPP
#define LOCKFREE_BUFFER_HPP
#include <atomic>
#include <iostream>
#include <cassert>
#include <cmath>

template <class T>
struct StampedElement
{
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
        if (size_ == cap_)
        {
            start_ = (start_ + 1ull) % cap_;
            size_--;
        }
        buffer_[(start_ + size_++) % cap_] = el;
    }

    // if size == 0 causes UB
    size_t binsearch(uint64_t ts, size_t first, size_t len) const
    {
        size_t start = first;
        while (len > 2ull)
        {
            size_t half = len >> 1;
            size_t mid = (first + half) % cap_;
            // if mid was changed during bisect in case if should exit search and rerun it
            if (was_changed(mid, start))
                return (mid + 1ull) % cap_;
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
        size_t second = (first + 1) % size_;
        size_t ts0 = buffer_[first].ts;
        size_t ts1 = buffer_[second].ts;
        size_t choice = first;
        if (ts0 < ts1 && ts > ts0)
            choice = second;
        return choice;
    }

    ElType binsearch(uint64_t ts) const
    {
        size_t start = start_;
        size_t first = start, len = size_;
        size_t choice = binsearch(ts, first, len);
        // elements from start to start_ where changed to bigger ones in ascending order
        while (was_changed(choice, start))
        {
            choice = binsearch(ts, first, len);
        }
        return buffer_[choice];
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
    bool was_changed(size_t i, size_t prev_start) const
    {
        if (start_ >= prev_start)
            return i < start_ & i >= prev_start; // edge case: during consuming whole ring was changed, s.t. start_ came back to prev_start
        return i < start_ | i >= prev_start;
    }

private:
    ElType *buffer_;
    size_t cap_;
    std::atomic<size_t> start_ = 0, size_ = 0;
};

#endif

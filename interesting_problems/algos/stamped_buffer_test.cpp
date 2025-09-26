#include "stamped_buffer.hpp"

int main()
{
    StampedRingBuffer<char> rb(6);
    for(int i = 0; i < 50; ++i)
    {
        rb.add({(uint64_t)i, 'a'});
        rb.print(true);
        StampedElement<char> r = rb.binsearch(11);
        std::cout << r.ts << std::endl;
    }

    return 0;
}
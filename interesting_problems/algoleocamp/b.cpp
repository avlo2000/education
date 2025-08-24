#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>
#include <map>


using namespace std;

long long trg(long long u, long long d, long long i) {
    return floor((i + u + 1) / (i + d + 1));
}

long long next_i(long long u, long long d, long long i, long long n) {
    long long lo = i;
    long long hi = i + n;
    long long a = trg(u, d, lo);
    while (lo < hi) {
        long long mid = (lo + hi) / 2;
        if (a > trg(u, d, mid))
            hi = mid;
        else
            lo = mid + 1;
    }
    return lo;
}

int main()
{
    long long n, u, d;
    cin >> n;
    cin >> u;
    cin >> d;
    long long sum = 0;
    long long i0 = 0;
    for(; i0 < min(10000000ll, n); i0++) {
        sum += trg(u, d, i0);
    }
    for(; i0 < n;) {
        long long i1 = next_i(u, d, i0, n - 1);
        long long dist = min(n, i1);
        sum += trg(u, d, i0) * (dist - i0);
        i0 = dist;
    }
    cout << sum << endl;
}

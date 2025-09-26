#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>
#include <bitset>
#include <set>

using namespace std;

long long reverse_bits(long long n) {
    long long ans = 0;

    while (n > 0) {
        ans <<= 1;
        if ((n & 1) == 1) ans |= 1;
        n >>= 1;
    }
    return ans;
}


std::vector<long long> pals;
void gen()
{
    std::vector<long long> pals_odd;
    std::vector<long long> pals_even;
    pals_odd.push_back(0);
    for(int i = 1; i < 10000000; ++i) {
        long long head_odd = i;
        long long tail = reverse_bits(head_odd);
        int gsb = log2(head_odd);
        long long res_even = tail;
        long long res_odd = tail;
        long long head_even = head_odd;
        head_odd <<= gsb;
        head_even <<= (gsb + 1);
        res_odd |= head_odd;
        res_even |= head_even;
        pals_odd.push_back(res_odd);
        pals_even.push_back(res_even);
    }
    pals.reserve(pals_odd.size() + pals_even.size());
    merge(pals_odd.begin(), pals_odd.end(),
           pals_even.begin(), pals_even.end(),
           back_inserter(pals));
    for(int i = 0; i < 100; ++i) cout << pals[i] << endl;
}

int countBinaryPalindromes(long long n) {
    if(pals.empty()) gen();
    auto it = lower_bound(pals.begin(), pals.end(), n);
    return it - pals.begin();
}

int main()
{
    cout << countBinaryPalindromes(9) << endl;
    return 0;
}
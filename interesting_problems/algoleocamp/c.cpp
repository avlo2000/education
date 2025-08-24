#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>
#include <map>

using namespace std;

long long dp(const std::vector<long long>& p, int k) {
    if(k == p.size() - 1) {
        return p[k];
    }
    long long total = 0;
    long long res = dp(p, k + 1) % 998244353;
    total += res * p[k];
    total += res;
    return total;
}

int main()
{
    int n;
    cin >> n;
    vector<long long> p(n);
    for(long long& el : p)
       cin >> el;
    p.push_back(1);
    cout << (dp(p, 0) - 1) % 998244353 << endl;
}

// 2
// 998244352 84312
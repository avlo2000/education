
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <list>

using namespace std;

vector<int> trial_division(int n)
{
    vector<int> factorization;
    for (int d = 2; d * d <= n; d++)
    {
        while (n % d == 0)
        {
            factorization.push_back(d);
            n /= d;
        }
    }
    if (n > 1)
        factorization.push_back(n);
    return factorization;
}

int diff(const vector<int> &res)
{
    int mn = 99999999;
    int mx = 0;
    for (int r : res)
    {
        mn = min(mn, r);
        mx = max(mx, r);
    }
    return mx - mn;
}

size_t hashfn(const vector<int>& res)
{
    size_t h = 0;
    size_t p = 1;
    for(int r : res)
    {
        h += r * p;
        p *= 17;
    }
    return h % 100009;
}
vector<int> memo[100009] = {vector<int>()};

vector<int> dp(vector<int> res, const vector<int> &factors, int idx, int n, int k)
{
    if (idx == factors.size()) {
        memo[hashfn(res)] = res;
        return res;
    }
    if(!memo[hashfn(res)].empty()) return memo[hashfn(res)];

    vector<int> cand0(k, 1);
    cand0.back() = n;
    for (int i = 0; i < res.size(); i++)
    {
        res[i] *= factors[idx];
        vector<int> cand1 = dp(res, factors, idx + 1, n, k);
        memo[hashfn(res)] = cand1;
        res[i] /= factors[idx];
        if (diff(cand0) > diff(cand1))
            cand0 = cand1;
    }
    memo[hashfn(cand0)] = cand0;
    return cand0;
}

vector<int> minDifference(int n, int k)
{
    vector<int> res(k, 1);
    auto factors = trial_division(n);
    return dp(res, factors, 0, n, k);
}

int main()
{
    auto res = minDifference(72072, 5);
    for (auto f : res)
        cout << f << " ";
    cout << endl;
    return 0;
}
// 8 9 7 11 13 
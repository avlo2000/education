#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <bitset>

using namespace std;

vector<int> productQueries(int n, vector<vector<int>>& queries) {
    bitset<40> b(n);
    vector<int64_t> a;
    for(int i = 0; i < 40; i++) {
        if(b.test(i))  {
            a.push_back(1 << i);
        }
    }
    vector<int> res;
    int mod = std::pow(10, 9) + 7;
    for(vector<int> q : queries) {
        int l = q[0];
        int r = q[1];
        int64_t prod = 1;
        for (int i = l; i <=r; i++)
        {
            prod *= a[i] % mod;
            prod %= mod;
        }
        res.push_back(prod);
    }
    return res;
}

int main()
{
    vector<vector<int>> queries;
    queries.push_back({0, 1});
    queries.push_back({2, 2});
    queries.push_back({0, 3});
    productQueries(15, queries);
    return 0;
}

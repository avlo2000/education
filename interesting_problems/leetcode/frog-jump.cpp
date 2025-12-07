
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <list>
#include <stack>
#include <cstring>

using namespace std;

map<pair<int, int>, bool> memo;
bool dp(int i, int k, const set<int>& stones) {
    if(memo.find({i, k}) != memo.end()) return memo[{i, k}];
    if(i == *stones.rbegin()) 
        return true;
    if(stones.find(i) == stones.end() || k <= 0) return false;
    bool res = false;
    res |= dp(i + k, k + 1, stones);
    res |= dp(i + k, k, stones);
    res |= dp(i + k, k - 1, stones);
    memo[{i, k}] = res;
    return res;
}
bool canCross(const vector<int>& stones) {
    set<int> st(stones.begin(), stones.end());
    return dp(0, 1, st);
}

int main()
{
    cout << canCross({0,1,3,6,7}) << endl;
    return 0;
}

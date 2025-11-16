
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

int memo[303][303] = {-1};

int dfs(int i, int j, const vector<int>& nums) {
    if(memo[i][j] != -1) return memo[i][j];
    int res = 0;
    for(int k = i + 1; k < j; ++k) {
        res = max(res, nums[i] * nums[k] * nums[j] + 
        dfs(i, k, nums) + dfs(k, j, nums));
    }
    memo[i][j] = res;
    return res;
}

int maxCoins(const vector<int>& nums) {
    memset(memo, -1, sizeof(memo));
    vector<int> padded(nums.size() + 2, 1);
    for(int i = 0; i < nums.size(); ++i) padded[i + 1] = nums[i];
    return dfs(0, padded.size() - 1, padded);
}

int main()
{
    // 3,1,5,8
    cout << maxCoins({3,1,5,8}) << endl;
    return 0;
}

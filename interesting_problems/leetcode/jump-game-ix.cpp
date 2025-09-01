
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>

using namespace std;

int dp(const vector<int>& nums, int i, vector<bool> was)
{

    if(was[i]) return nums[i];
    int mx = nums[i];
    was[i] = true;
    cout << i << " | ";
    for(bool w : was) cout << w << " ";
    cout << endl;
    for(int j = 0; j < nums.size(); j++)
    {
        if(j > i && nums[j] < nums[i]) mx = max(mx, dp(nums, j, was));
        if(j < i && nums[j] > nums[i]) mx = max(mx, dp(nums, j, was));
    }
    return mx;
}

vector<int> maxValue(const vector<int>& nums) {
    int n = nums.size(), min_r = 99999999;
    vector<int> res{nums[0]};
    for (int i = 1; i < n; ++i)
        res.push_back(max(nums[i], res.back()));
    for (int i = n - 1; i >= 0; min_r = min(min_r, nums[i--]))
        if (res[i] > min_r)
            res[i] = res[i + 1];
    return res;
}

int main() {
    auto res = maxValue({1, 2, 3, 4, 5, 6});
    for(int r : res) cout << r << " ";
    cout << endl;
    return 0;
}
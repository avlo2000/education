
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <list>
#include <stack>
#include <cstring>
#include <numeric>

using namespace std;

long long mod = 1000000007;

long long score(vector<int> nums, int k) {
    for(int i = 0; i < nums.size(); ++i) 
        if(nums[i] % k != 0) nums[i] *= -1; 
    long long mx = -1e9;
    long long res = nums[0];           
    long long maxEnding = nums[0]; 
    for(int i = 1; i < nums.size(); ++i) {
        maxEnding = max((long long)nums[i], maxEnding + nums[i]);
        res = max(res, maxEnding);
    }
    return res;
}

set<int> divs(const vector<int>& nums) {
    set<int> d;
    d.insert(2);
    for(int n : nums) {
        if (n != 1)
            d.insert(n);
        for(int i = 3; i * i <= n; ++i) {
            if(n % i == 0) {
                d.insert(i);
                d.insert(n / i);
            }
        }
    }
    return d;
}

int divisibleGame(const vector<int>& nums) {
    long long mx = -99999999;
    long long bestk = 2;
    set<int> ks = divs(nums);
    for(int k : ks) {
        int s = score(nums, k);
        if(s > mx) {
            mx = s;
            bestk = k;
        }
    }
    long long r = mx * bestk;
    if (r < 0) r = mod + r;
    return (int)(r % mod);
}

int main()
{
    std::cout << divisibleGame({4,6,15,20,7}) << endl;
    return 0;
}

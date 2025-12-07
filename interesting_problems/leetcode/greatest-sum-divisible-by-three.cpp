
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

int bf(int i, int sum, const vector<int>& nums) {
    if(i == nums.size()) 
        return sum % 3 == 0 ? sum : -1;
    return max(
        bf(i + 1, sum + nums[i], nums),
        bf(i + 1, sum, nums)
    );
}
int maxSumDivThree(const vector<int>& nums) {
    vector<int> mod1, mod2;
    int res = 0;
    for(int n : nums) {
        if(n % 3 == 0) res += n;
        if(n % 3 == 1) mod1.push_back(n);
        if(n % 3 == 2) mod2.push_back(n);
    }
    vector<int> rest;
    sort(mod1.rbegin(), mod1.rend());
    sort(mod2.rbegin(), mod2.rend());
    int i = 0;
    for(; i < (int)mod1.size() - 4; i += 3) {
        res += mod1[i] + mod1[i + 1] + mod1[i + 2];
    }
    for(; i < mod1.size(); i++) {
        rest.push_back(mod1[i]);
    }

    i = 0;
    for(; i < (int)mod2.size() - 4; i += 3) {
        res += mod2[i] + mod2[i + 1] + mod2[i + 2];
    }
    for(; i < mod2.size(); i++) {
        rest.push_back(mod2[i]);
    }
    return res + bf(0, 0, rest);
}

int main()
{
    cout << maxSumDivThree({2,19,6,16,5,10,7,4,11,6}) << endl;
    return 0;
}

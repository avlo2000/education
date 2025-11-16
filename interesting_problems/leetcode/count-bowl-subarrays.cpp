
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <list>
#include <stack>

using namespace std;

long long bowlSubarrays(const vector<int>& nums) {
    stack<int> s;
    s.push(nums[0]);
    long long cnt = 0;
    for(int i = 1; i < nums.size(); ++i) {

        while (!s.empty() && s.top() < nums[i])
        {
            int prev_top = s.top();
            s.pop();
            if(!s.empty())
                cnt += prev_top < s.top();
        }
        s.push(nums[i]);
    }
    return cnt;
}

int main()
{
    cout << bowlSubarrays({5,0,4,2,5}) << endl;
}

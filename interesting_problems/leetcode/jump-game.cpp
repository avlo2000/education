
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>

using namespace std;

bool canJump(vector<int> nums) {
    int mx_len = 0;
    for(int i = 0; i < nums.size(); i++)
    {
        if(i > mx_len) return false;
        mx_len = max(mx_len, i + nums[i]);
    }
    return true;
}

int main() {
    auto res = canJump({1, 2, 3, 4, 5, 6});
    cout << res << " ";
    cout << endl;
    return 0;
}
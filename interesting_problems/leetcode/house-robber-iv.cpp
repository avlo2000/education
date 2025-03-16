#include <iostream>
#include <vector>
#include <algorithm>
#include <list>

using namespace std;

int max_possible_k(vector<int> nums, int m){
    sort(nums.begin(), nums.begin() + m);
    int in_seq = 0;
    int k = 0;
    for (int i = 0; i < m; i++) {
        if (i == 0 || nums[i] == nums[i - 1] + 1) {
            in_seq++;
        } else {
            k += (in_seq + 1) / 2;
            in_seq = 1;
        }
        if (i == m - 1) {
            k += (in_seq + 1) / 2;
        }
    }
    return k;
};

int minCapability(vector<int> nums, int k)
{
    vector<pair<int, int>> nums_with_index;
    for(int i = 0; i < nums.size(); i++){
        nums_with_index.push_back({nums[i], i});
    }
    sort(nums_with_index.begin(), nums_with_index.end());
    vector<int> locs;
    for(int i = 0; i < nums.size(); i++){
        locs.push_back(nums_with_index[i].second);
    }
    int m_low = 0;
    int m_high = nums.size();
    while(m_low < m_high){
        int m = m_low + (m_high - m_low) / 2;
        if(max_possible_k(locs, m) >= k){
            m_high = m;
        } else {
            m_low = m + 1;
        }
    }
    int max_num = -1;
    for (int i = 0; i < m_low; i++) {
        max_num = max(max_num, nums_with_index[i].first);
    }
    return max_num;
}

int main()
{
    int k = 2;
    vector<int> nums = {2,3,5,9};
    cout << minCapability(nums, k) << endl;
    return 0;
}

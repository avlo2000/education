#include <iostream>
#include <vector>
#include <algorithm>
#include <list>

using namespace std;

int maxSatisfaction(vector<int> satisfaction) {
    sort(satisfaction.begin(), satisfaction.end());
    int mx_sat = 0;
    for(int i = 0; i < satisfaction.size(); i++) {
        int sat = 0;
        for(int j = i; j < satisfaction.size(); j++) {
            sat += satisfaction[j] * (j - i + 1);
        }
        mx_sat = max(mx_sat, sat);
    }
    return mx_sat;
}

int main()
{
    vector<int> nums = {-1,-8,0,5,-7};
    cout << maxSatisfaction(nums) << endl;
    return 0;
}

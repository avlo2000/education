
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <bitset>

using namespace std;

struct par {
    unsigned int val, idx;
    bool operator<(const par& other) const {
        return val < other.val;
    }
};

unsigned int reverse_bits(unsigned int n) {
	unsigned int ans = n;
    unsigned int full = 1;
	while (n > 1) {
        full <<= 1;
        full++;
		n >>= 1;

	}
	return ans ^ full;
}

long long maxProduct(const vector<unsigned int>& nums) {
    vector<par> inv;
    for(unsigned int i = 0; i < nums.size(); ++i) {
        inv.push_back({reverse_bits(nums[i]), i});
        cout << bitset<32>(reverse_bits(nums[i])) << endl;
        cout << bitset<32>(nums[i]) << endl;
        cout << endl;
    }
    sort(inv.begin(), inv.end());
    long long mx_prod = 0;
    for(int i = 0; i < nums.size(); ++i) {
        par t{nums[i], 0};
        auto from = lower_bound(inv.begin(), inv.end(), t);
        auto to = upper_bound(inv.begin(), inv.end(), t);
        for(auto it = from; it != to; it ++) {
            if(it->idx != i) {
                mx_prod = max((long long)(it->val * nums[i]), mx_prod);
            }
        }
    }
    return mx_prod;
}

int main() {
    auto res = maxProduct({1,2,3,4,5,6,7});
    cout << res << " ";
    cout << endl;
    return 0;
}
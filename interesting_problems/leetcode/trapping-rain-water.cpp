
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <list>
#include <stack>

using namespace std;

int trap(const vector<int>& height) {
    stack<int> s;
    s.emplace(height[0]);
    int cnt = 0;
    int water = 0;
    for(int i = 1; i < height.size(); ++i) {
        int mx_in = -1;
        while (!s.empty() && s.top() < height[i])
        {
            int prev_top = s.top();
            s.pop();
            if(!s.empty() && prev_top < s.top()) {
                // means that we have end of bowl
                cout << "[" << prev_top << "]";
            }
            else{
                cout << prev_top << " ";
            }
            mx_in = max(mx_in, prev_top);
        }
        s.emplace(height[i]);
        cout << endl;
    }
    cout << endl;
    return cnt;
}

int main()
{
    cout << trap(vector({5,0,3,5})) << endl;
}

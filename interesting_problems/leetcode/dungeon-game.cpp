
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

bool can_cross(int i, int j, int hp, const vector<vector<int>>& d)
{
    hp -= d[i][j];
    if(i == d.size() - 1 && j == d[0].size() - 1) return true;
    if(hp <= 0) return false;
    bool can = false;
    if(i + 1 < d.size()) can |= can_cross(i + 1, j, hp, d);
    if(j + 1 < d[0].size()) can |= can_cross(i, j + 1, hp, d);
    return can;
}

int calculateMinimumHP(const vector<vector<int>>& d) {
    int low = 1;
    int high = 200 * 200 * 1000;
    while(high > low) {
        int mid = (low + high) / 2;
        if(can_cross(0, 0, mid, d))
            high = mid;
        else
            low = mid + 1;
    }
    return low;
}

int main()
{
    // [[-2,-3,3],[-5,-10,1],[10,30,-5]]
    cout << calculateMinimumHP({{-2,-3,3},{-5,-10,1},{10,30,-5}}) << endl;
    return 0;
}

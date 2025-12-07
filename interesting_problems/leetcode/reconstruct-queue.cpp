#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>
#include <stack>
#include <queue>
#include <map>
#include <set>

using namespace std;

bool cmp(const vector<int>& a, const vector<int>& b) {
    if(a[0] == b[0]) return a[1] > b[1];
    return a[0] < b[0];
}

vector<vector<int>> reconstructQueue(vector<vector<int>> people) {
    vector<int> idx(people.size(), -1);
    sort(people.begin(), people.end(), cmp);
    for(int i = 0; i < people.size(); i++) {
        int cnt = people[i][1];
        int ii = 0;
        while (cnt != 0 || idx[ii] > -1)
        {
            if(idx[ii] == -1){
                 cnt--;
            }
            ii++;
        }
        idx[ii] = i;
    }
    vector<vector<int>> res = people;
    for(int i = 0; i < people.size(); ++i)
        res[i] = people[idx[i]];
    return res;
}

int main()
{
    vector<vector<int>> buildings = {{7,0},{4,4},{7,1},{5,0},{6,1},{5,2}};
    vector<vector<int>> skyline = reconstructQueue(buildings);
    for(auto p : skyline) {
        cout << "[" << p[0] << "," << p[1] << "] ";
    }
    cout << endl;
    // [[2,10],[3,15],[7,12],[12,0],[15,10],[20,8],[24,0]]
    return 0;
}

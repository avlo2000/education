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

struct event2d 
{
    int h, x, type;
    int id;
};

bool cmp(const event2d& a, const event2d& b) {
    if(a.x != b.x)
        return a.x < b.x;
    if (a.type != b.type)
        return a.type < b.type;
    if (a.type == 0)
        return a.h > b.h;
    return a.h < b.h;
}

vector<vector<int>> getSkyline(vector<vector<int>>& buildings) {
    vector<event2d> events;
    int id = 0;
    for(auto b : buildings) {
        events.push_back(event2d{b[2], b[0], 0, id});
        events.push_back(event2d{b[2], b[1], 1, id});
        id++;
    }
    sort(events.begin(), events.end(), &cmp);
   
    int h = 0;
    multiset<pair<int, int>, std::greater<pair<int, int>>> h_id;
    vector<vector<int>> res;
    for(int i = 0; i < events.size(); ++i) {
        if(events[i].type == 0) {
            h_id.insert({events[i].h, events[i].id});
            if(h < events[i].h) {
                h = events[i].h;
                // cout << events[i].x << " " << h << endl;
                res.push_back({events[i].x, h});
            }
        }
        else {
            h_id.erase({events[i].h, events[i].id});
            int lower_h = h_id.begin()->first;
            if(h == events[i].h && h > lower_h) {
                h = lower_h;
                // cout << events[i].x << " " << h  << endl;
                res.push_back({events[i].x, h});
            }
        }
    }
    return res;
}

int main()
{
    // vector<vector<int>> buildings = {{0,2,3},{2,5,3}};
    vector<vector<int>> buildings = {{1,2,1},{1,2,2},{1,2,3}};
    // vector<vector<int>> buildings = {{2,9,10},{3,7,15},{5,12,12},{15,20,10},{19,24,8}};
    vector<vector<int>> skyline = getSkyline(buildings);
    for(auto p : skyline) {
        cout << "[" << p[0] << "," << p[1] << "] ";
    }
    cout << endl;
    // [[2,10],[3,15],[7,12],[12,0],[15,10],[20,8],[24,0]]
    return 0;
}

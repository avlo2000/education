#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>
#include <stack>
#include <queue>
#include <map>

using namespace std;

struct event 
{
    int h, x, type;
    int id;
};

bool cmp(const event& a, const event& b) {
    return a.x < b.x;
}

vector<vector<int>> getSkyline(vector<vector<int>>& buildings) {
    vector<event> events;
    int id = 0;
    for(auto b : buildings) {
        events.push_back(event{b[2], b[0], 0, id});
        events.push_back(event{b[2], b[1], 1, id});
        id++;
    }
    sort(events.begin(), events.end(), &cmp);
   
    int h = events[0].h;
    map<int, int> heights;
    vector<vector<int>> res;
    for(int i = 1; i < events.size(); ++i) {
        if(events[i].type == 0) {
            heights[events[i].id] = events[i].h;
            if(h < events[i].h) {
                h = events[i].h;
            }
            h = max(h, events[i].h);
        }
        else {

        }
    }
    return {};
}

int main()
{
    vector<vector<int>> buildings = {{2,9,10},{3,7,15},{5,12,12},{15,20,10},{19,24,8}};
    vector<vector<int>> skyline = getSkyline(buildings);
    for(auto p : skyline) {
        cout << "[" << p[0] << "," << p[1] << "] ";
    }
    cout << endl;
    return 0;
}

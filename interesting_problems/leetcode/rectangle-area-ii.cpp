#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

using namespace std;

struct event2d
{
    long long x, y0, y1;
    int id;
    int type;
};

struct event1d
{
    long long x;
    int id;
    int type;
};

bool cmp2d(const event2d& e0, const event2d& e1) 
{
    return e0.x < e1.x;
}

bool operator<(const event1d& e0, const event1d& e1) 
{
    return e0.x < e1.x;
}

long long unio(const map<int, event2d>& current)
{
    vector<event1d> events;
    for(auto&[_, e] : current) {
        events.push_back({e.y0, e.id, 0});
        events.push_back({e.y1, e.id, 1});
    }
    sort(events.begin(), events.end());
    bool active = false;

    int cnt = 0;
    int last_x = 0;
    long long len = 0;
    for(const auto& e : events) {
        if(cnt != 0)
            len += e.x - last_x;
        last_x = e.x;
        if(e.type == 0) cnt++;
        else cnt--;
    }
    return len;
}

long long rectangleArea(const vector<vector<int>>& rectangles) {
    int mod = 1000000000 + 7;
    vector<event2d> events;
    int id = 0;
    for(auto r : rectangles) {
        long long x0 = r[0];
        long long y0 = r[1];
        long long x1 = r[2];
        long long y1 = r[3];
        events.push_back({x0, y0, y1, id, 0});
        events.push_back({x1, y0, y1, id, 1});
        id++;
    }
    sort(events.begin(), events.end(), &cmp2d);
    long long prev_x = 0;
    long long area = 0;
    map<int, event2d> current;
    for(event2d e : events) {
        long long line = unio(current);
        area += line * (e.x - prev_x);
        area %= mod;
        if(e.type == 0) 
            current[e.id] = e;
        else 
            current.erase(e.id);

        prev_x = e.x;
    }
    return area;
}

int main()
{
    // [[0,0,2,2],[1,0,2,3],[1,0,3,1]]
    cout << rectangleArea({{0,0,1000000000,1000000000}}) << endl;
    return 0;
}

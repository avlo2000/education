#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>
#include <bitset>
#include <queue>

using namespace std;

constexpr int kMxTrgs = 13;
typedef bitset<kMxTrgs> bs;

struct Node
{
    int idx;
    int dist;
    bs used;
};

int shortest_path(const vector<vector<int>>& graph, int from)
{
    int n = graph.size();
    int all_used = (1 << graph.size()) - 1;
    bs all_used_bs(all_used);
    vector<vector<int>> dists;
    for(int i = 0; i < n; i++) dists.push_back(vector<int>(all_used + 1, INT_MAX));
    bs used = 0;
    used[from] = true;
    dists[0][used.to_ulong()] = 0;
    Node start;
    start.idx = from;
    start.used = used;
    start.dist = 0;
    queue<Node> q;
    q.push(start);
    while (!q.empty())
    {
        Node curr = q.front();
        q.pop();
        for(int v : graph[curr.idx])
        {
            bs new_used = curr.used;
            new_used[v] = true;
            int new_dist = curr.dist + 1;
            if(new_dist < dists[v][new_used.to_ulong()])
            {
                Node next;
                next.idx = v;
                next.dist = new_dist;
                next.used = new_used;
                dists[v][new_used.to_ulong()] = new_dist;
                q.push(next);
            }
        }
    }
    int min_dst = INT_MAX;
    for(int i = 0; i < n; i++) {
        min_dst = min(min_dst, dists[i][all_used_bs.to_ulong()]);
    };
    return min_dst;
}

int shortestPathLength(const vector<vector<int>>& graph) {
    int min_dst = INT_MAX;
    for(int i = 0; i < graph.size(); i++)
    {
        min_dst = min(min_dst, shortest_path(graph, i));
    }
    return min_dst;
}

int main()
{
    // cout << shortestPathLength({{1},{0,2,4},{1,3,4},{2},{1,2}}) << endl;
    cout << shortestPathLength({{1, 2, 3},{0},{0},{0}}) << endl;
    return 0;
}



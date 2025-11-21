
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <string>
#include <map>
#include <set>

using namespace std;

void dfs(map<string, set<string>>& graph, const string& curr, vector<string>& path, int n_verts, set<pair<string, string>>& visited)
{
    for(string to : graph[curr]) {
        if(visited.find({curr, to}) != visited.end()) continue;
    
        visited.insert({curr, to});
        dfs(graph, to, path, n_verts, visited);
    }
    path.push_back(curr);
}


vector<string> findItinerary(vector<vector<string>>& tickets) {
    map<string, set<string>> graph;
    int n_verts = tickets.size();
    for(auto t : tickets) {
        string t0 = t[0];
        string t1 = t[1];
        if(graph.find(t0) == graph.end()) {
            graph[t0] = set<string>();
        }
        graph[t0].insert(t1);
    }
    set<pair<string, string>> vis;
    vector<string> path;
    dfs(graph, "JFK", path, n_verts, vis);
    return path;
}

int main()
{
    vector<vector<string>> tickets = {{"JFK","SFO"},{"JFK","ATL"},{"SFO","ATL"},{"ATL","JFK"},{"ATL","SFO"}};
    // vector<vector<string>> tickets = {{"MUC","LHR"},{"JFK","MUC"},{"SFO","SJC"},{"LHR","SFO"}};
    vector<string> itinerary = findItinerary(tickets);
    cout << endl;
    for(auto s : itinerary) {
        cout << s << " ";
    }
    cout << endl;
    return 0;
}

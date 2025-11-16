
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <list>
#include <stack>

using namespace std;

int dfs(const vector<vector<int>> &m, vector<vector<int>> &dist,
        vector<vector<int>> &was, int i, int j)
{
    int res = dist[i][j];
    if (was[i][j] == 0)
        return res;
    was[i][j] = 1;
    if (i != 0 && m[i - 1][j] < m[i][j])
        res = max(res, dfs(m, dist, was, i - 1, j) + 1);
    if (j != 0 && m[i][j - 1] < m[i][j])
        res = max(res, dfs(m, dist, was, i, j - 1) + 1);
    if (i + 1 < dist.size() && m[i + 1][j] < m[i][j])
        res = max(res, dfs(m, dist, was, i + 1, j) + 1);
    if (j + 1 < dist[0].size() && m[i][j + 1] < m[i][j])
        res = max(res, dfs(m, dist, was, i, j + 1) + 1);
    dist[i][j] = res;
    return res;
}
int longestIncreasingPath(const vector<vector<int>> &matrix)
{
    vector<vector<int>> dist = matrix;
    vector<vector<int>> was = matrix;
    for (int i = 0; i < was.size(); ++i)
    {
        for (int j = 0; j < was[0].size(); ++j)
        {
            was[i][j] = 1;
            dist[i][j] = 1;
        }
    }
    for (int i = 0; i < was.size(); ++i)
    {
        for (int j = 0; j < was[0].size(); ++j)
        {
            dfs(matrix, dist, was, i, j);
        }
    }

    int mx = 0;
    for (int i = 0; i < was.size(); ++i)
    {
        for (int j = 0; j < was[0].size(); ++j)
        {
            mx = max(mx, dist[i][j]);
        }
    }
    return mx;
}

int main()
{
    // 3 4 5
    // 3 2 6
    // 2 2 1
    vector<vector<int>> m = {
        {9, 9, 4},
        {6, 6, 8},
        {2, 1, 1}};
    cout << longestIncreasingPath(m) << endl;
}

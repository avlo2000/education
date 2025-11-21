
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <cstring>

using namespace std;

int16_t memo[203][203][1003];

int16_t go(
    const vector<vector<int>> &grid, 
    int i, int j, int k)
{
    int n = grid.size();
    int m = grid[0].size();
    if(grid[i][j] > 0) k--;
    if(k < 0) {
        return INT16_MIN;
    }
    if(memo[i][j][k] != -1) return memo[i][j][k];

    int16_t s = grid[i][j];
    if(i == n - 1 && j == m - 1) {
        memo[i][j][k] = s;
        return s;
    }
    int16_t mx_score = INT16_MIN;
    if(i != n - 1) mx_score = max(mx_score, (int16_t)(s + go(grid, i + 1, j, k)));
    if(j != m - 1) mx_score = max(mx_score, (int16_t)(s + go(grid, i, j + 1, k)));
    memo[i][j][k] = mx_score;
    return mx_score;
}

int16_t maxPathScore(const vector<vector<int>> &grid, int k)
{
    memset(memo, -1, sizeof(memo));
    return max((int16_t)-1, go(grid, 0, 0, k));
}

int main()
{
    vector<vector<int>> grid{{0, 1}, {2, 0}};
    int k = 1;
    int res = maxPathScore(grid, k);
    cout << res << endl;
    return 0;
}
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>

using namespace std;

int dp(const vector<vector<int>>& grid, int i, int j, vector<vector<bool>> used, int n_tot) {
    if(used[i][j] || grid[i][j] == -1) 
        return 0;
    if(grid[i][j] == 2 && n_tot == 0) 
        return 1;
    if(grid[i][j] == 2 && n_tot != 0) 
        return 0;

    int n = grid.size();
    int m = grid[0].size();
    int res = 0;
    if(i >= 1) {
        used[i][j] = true;
        res += dp(grid, i - 1, j, used, n_tot - 1);
        used[i][j] = false;
    }
    if(i < n - 1) {
        used[i][j] = true;
        res += dp(grid, i + 1, j, used, n_tot - 1);
        used[i][j] = false;
    }
    if(j >= 1) {
        used[i][j] = true;
        res += dp(grid, i, j - 1, used, n_tot - 1);
        used[i][j] = false;
    }
    if(j < m - 1) {
        used[i][j] = true;
        res += dp(grid, i, j + 1, used, n_tot - 1);
        used[i][j] = false;
    }
    return res;
}

int uniquePathsIII(const vector<vector<int>>& grid) {
    int n = grid.size();
    int m = grid[0].size();
    int n_tot = 0;
    int s_i, s_j;
    for(int i = 0; i < n; ++i)
        for(int j = 0; j < m; ++j) {
            if(grid[i][j] == 0 || grid[i][j] == 2)
                n_tot++;
            if(grid[i][j] == 1) {
                s_i = i; s_j = j;
            }
        }
    vector<vector<bool>> used(n, vector<bool>(m, false));
    return dp(grid, s_i, s_j, used, n_tot);
}

int main()
{
    cout << uniquePathsIII({
        {1,0,0,0}, {0,0,0,0}, {0,0,2,-1}
    });
    return 0;
}

#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>

using namespace std;

int memo[31][5000] = {-1};
int dp(int n, int k, int trg)
{
    if(memo[n][trg] != -1)
    {
        return memo[n][trg];
    }
    if(trg == 0)
    {
        memo[n][trg] = 1;
        return 1;
    }
    if(n == 0)
    {
        memo[n][trg] = 0;
        return 0;
    }
    int res = 0;
    for(int i = 1; i <= k; i++)
    {
        res += dp(n - 1, k, trg - i) % 1000000007;
    }
    memo[n][trg] = res;
    return res;
}

int numRollsToTarget(int n, int k, int target) {
    for (int i = 0; i < 31; i++)
    {
        for (int j = 0; j < 5000; j++)
        {
            memo[i][j] = -1;
        }
    }
    return dp(n, k, target);
}

int main()
{
    cout << numRollsToTarget(2, 6, 7) << endl;
    return 0;
}
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>

using namespace std;

int numDistinct(string s, string t)
{
    int n = s.size();
    int m = t.size();
    vector<vector<int>> dp(m, vector<int>(n, 0));

    dp[0][0] = (s[0] == t[0]);
    for (int i = 1; i < n; ++i)
    {
        dp[0][i] = dp[0][i - 1] + (s[i] == t[0]);
    }
    for (int j = 1; j < m; ++j)
        for (int i = j; i < n; ++i)
        {
            dp[j][i] = dp[j][i - 1];
            if (s[i] == t[j])
            {
                dp[j][i] = dp[j - 1][i - 1] + dp[j][i - 1];
            }
        }
    return dp.back().back();
}

int main()
{
    // numDistinct("rabbbit", "rabbit");
    numDistinct("babgbag", "bag");
    return 0;
}
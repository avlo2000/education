#include <iostream>
#include <vector>
#include <algorithm>
#include <list>

using namespace std;

int longestSubsequence(const vector<int> &a)
{
    vector<int> lds(a.size(), 1);
    vector<int> prev_idx(a.size());
    for (int i = 0; i < a.size(); i++)
        prev_idx[i] = i;
    for (int i = 0; i < a.size(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (abs(a[i] - a[j]) <= abs(a[j] - prev_idx[j]))
            {
                lds[i] = max(lds[j] + 1, lds[i]);
                prev_idx[i] = j;
            }
        }
        cout << lds[i] << " ";
    }
    cout << endl;
    int mx_n = -1;
    for (int l : lds)
        mx_n = max(mx_n, l);
    return mx_n;
}

int main()
{
    cout << longestSubsequence({8, 5, 9, 3});
    return 0;
}

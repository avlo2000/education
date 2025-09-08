#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>

using namespace std;

bool isMatch(string s, string p)
{
    int n = s.size();
    int m = p.size();
    int j = 0;
    int last_star = -1;
    int last_star_selection = -1;
    for (int i = 0; i < n;)
    {
        if (s[i] == p[j] || p[j] == '?')
        {
            j++;
            i++;
            continue;
        }
        if (p[j] == '*')
        {
            last_star = j;
            j++;
            last_star_selection = i;
            continue;
        }
        if (last_star != -1)
        {
            j = last_star + 1;
            last_star_selection++;
            i = last_star_selection;
            continue;
        }
        return false;
    }
    while (p[j] == '*' && j < p.size())
        j++;
    return j == p.size();
}

int main()
{
    cout << isMatch("adceb", "*a*b") << endl;
    return 0;
}

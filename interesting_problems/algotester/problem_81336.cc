#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>

using namespace std;

int main()
{
    int n;
    cin >> n;
    string s;
    cin >> s;
    int sum = 0;
    // for (int i = 0; i < n; i++)
    // {
    //     int d = s[i] - '0';
    //     int dec = 1;
    //     for(int k = 0; k < min(11, n - i); k++)
    //     {
    //         sum += d * dec * (i + 1);
    //         dec *= 10;
    //     }
    //     sum %= 998244353;
    // }
    // cout << sum << endl;
    for (int j = n - 1; j >= 0; j--)
    {
       int i = n - j;
       int d = s[i] - '0';
       int dec = 0;
       for(int k = 0; k < n - i; k++) dec += pow(10, k);
       sum += i * d * dec;
    }
    cout << sum << endl;
    return 0;
}
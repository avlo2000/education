#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>
#include <map>
#include <bitset>

using namespace std;

constexpr int asz = 'z' - 'a' + 1;
constexpr int sz = asz * 6;

int main()
{
    int n, k;
    std::cin >> n >> k;
    vector<bitset<sz>> bs(n);
    for (int i = 0; i < n; i++) {
        string s;
        cin >> s;
        bs[i] = 0;
        for(int j = 0; j < s.size(); j++) {
            char c = s[j];
            if(c != '?')
                bs[i].set(asz * i + (c - 'a'), true);
            else {
                for(int cc = 'a'; cc <= 'z'; cc++) {
                    bs[i].set(asz * i + (cc - 'a'), true);
                }
            }
        }
    }
    for(int i = 0; i < n; ++i) {
        cout << bs[i] << endl;
    }
}
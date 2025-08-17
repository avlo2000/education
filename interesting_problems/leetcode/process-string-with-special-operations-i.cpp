#include <iostream>
#include <vector>
#include <algorithm>
#include <list>

using namespace std;

char processStr(string s, long long k) {
    vector<long long> cnts;
    long long cnt = 0;
    for (char c : s) {
        if(c == '#') {
            cnt *= 2;
        }
        else if(c == '*') {
            cnt = max(0ll, cnt - 1);
        }
        else if(c != '%'){
            cnt++;
        }
        cnts.push_back(cnt);
    }
    for(int i = s.size() - 1; i >= 0; i--) {
        char c = s[i];
        long long n = cnts[i];
        if (n <= k)
            return '.';
        if(k == n - 1 && c >= 'a' && c <= 'z') {
            return c;
        }
        else if(c == '#') {
            if(k >= n / 2) k -= n / 2;
        }
        else if(c == '*') {
            if(k == n) k--;
        }
        else if(c == '%') {
            k = n - k - 1;
        }
    }
    return '.';
}

int main()
{
    cout << processStr("#m%e%*", 1) << endl;
    return 0;
}

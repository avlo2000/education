#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>

using namespace std;

bool is_happy(int n)
{
    string s = to_string(n);
    for(char c : s)
    {
        if (c != '4' && c != '7')
            return false;
    }
    return true;
}

int main()
{
    int n;
    cin >> n;
    int a = 1;
    int b = n - 1;
    while(is_happy(b) || is_happy(a))
    {
        a++;
        b--;
    }
    cout << a << " " << b << endl;
    return 0;
}
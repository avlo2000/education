#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>
#include <map>

using namespace std;

int main()
{
    int n;
    cin >> n;

    int non_null = -1;
    for(int i = 0; i < n; i++)
    {
        int p;
        cin >> p;
        if (p != 0) non_null = i + 1;
    }
    cout << non_null << endl;
}
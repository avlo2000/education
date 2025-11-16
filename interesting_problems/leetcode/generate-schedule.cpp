
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <list>
#include <stack>

using namespace std;

bool invalid(const vector<int>& a, vector<int>& b)
{
    return a[0] == b[0] || a[1] == b[0] || a[0] == b[1] || a[1] == b[1];
}

vector<vector<int>> generateSchedule(int m) {
    vector<vector<int>> all;
    for(int i = 0; i < m; ++i) {
        for(int j = 0; j < m; ++j) {
            if(i != j) all.push_back(vector<int>({i, j}));
        }
    }
    int n = all.size();
    int cnt = n * n * n * n;
    for(int i = 0; i < cnt; ++i) {
        int j = i + 1;
        while (invalid(all[i % n], all[(i + 1) % n]))
        {
            j++;
            j %= n;
            swap(all[(i + 1) % n], all[j]);
            cnt--;
            break;
        }
    }

    // for(int i = 0; i < all.size() - 1; ++i)
    //     if(invalid(all[i], all[i + 1])) return {};
    return all;
}

int main()
{
    auto res = generateSchedule(5);
    for(vector<int> r : res)
        cout << r[0] << " " << r[1] << endl;
}

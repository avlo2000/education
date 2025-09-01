
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <set>

using namespace std;

vector<int> recoverOrder(vector<int> order, vector<int>& friends) {
    vector<int> res;
    set<int> f(friends.begin(), friends.end());
    for (int o : order) {
        if (f.find(o) != f.end()) {
            res.push_back(o);
            f.erase(o);
        }
    }
    return res;
}

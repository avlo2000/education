#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <chrono>
#include <unordered_set>
#include <thread>

using namespace std;

vector<bool> is_prime (1e6 + 1, true);

void fill () {
    is_prime [0] = is_prime [1] = false;
    for (int i = 2; i * i <= 1e6; ++i) {
        if (is_prime [i]) {
            for (int j = i * i; j <= 1e6; j += i)
                is_prime [j] = false;
        }
    }
}

int minJumps(vector<int> nums)
{
    int n = 1e6 + 1;
    if (is_prime[0]) fill();

    std::unordered_map<int, std::vector<int>> val2idx;
    is_prime[1] = false;
    int max_n = -1;
    for (int i = 0; i < nums.size(); ++i)
    {
        max_n = max(max_n, nums[i]);
        val2idx[nums[i]].push_back(i);
    }

    queue<int> q;
    vector<int> dist(nums.size(), -1);
    dist[0] = 0;
    q.push(0);
    unordered_set <int> used;
    while (!q.empty())
    {
        int curr = q.front();
        q.pop();
        if(curr > 0 && dist[curr - 1] == -1) {
            dist[curr - 1] = dist[curr] + 1;
            q.push(curr - 1);
        }
        if(curr < nums.size() - 1 && dist[curr + 1] == -1) {
            dist[curr + 1] = dist[curr] + 1;
            q.push(curr + 1);
        }

        if(used.find(nums[curr]) != used.end() || !is_prime[nums[curr]]) {
            continue;
        }

        int p = nums[curr];
        for (int val_to = p; val_to <= max_n; val_to += p)
        {
            if(val2idx.find(p) == val2idx.end()) continue;
            for (int adj : val2idx[val_to])
            {
                if (dist[adj] != -1) continue;
                dist[adj] = dist[curr] + 1;
                q.push(adj);
            }
        }

        used.insert(nums[curr]);
    }

    return dist[nums.size() - 1];
}

int main()
{
    cout << minJumps({2887,2644,1263,1228,163,1121,2151,1163,1923,1456,2365,1600,2045,1528,2726,465,2224,612,1722,341,893,2041,2743,1921,547,444,2562,1563,187,1510,1149,2520,1007,1677,359,571,1155,1131,3116,145,1831,2479,1665,1016,609,1747,703,2944,2959,2056,302,2266,2233,1308,126,1483});
    return 0;
}

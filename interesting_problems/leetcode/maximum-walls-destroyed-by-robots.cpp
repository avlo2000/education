
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>

using namespace std;

int maxWalls(vector<int>& robots, vector<int>& distance, vector<int>& walls) {
    sort(walls.begin(), walls.end());
    
}

int main() {
    
    return 0;
}
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <set>

using namespace std;

// constexpr int N = 1000000001;

// int maxWalls(vector<int>& robots, vector<int>& distance, vector<int>& walls) {
//     vector<bool> mask(N, false);
//     for(int i = 0;)
// }

struct segm
{
    int left, right, idx, pos;
    bool operator<(const segm& other) const {
        return left < other.left;
    }
};

int maxWalls(vector<int>& robots, vector<int>& distance, vector<int>& walls) {
    int n = robots.size();
    vector<segm> segms;
    std::vector<std::pair<int, int>> robots_and_dists;
    for(int i = 0; i < n; ++i) robots_and_dists.emplace_back(robots[i], distance[i]);
    sort(robots_and_dists.begin(), robots_and_dists.end());
    for(int i = 0; i < n; ++i) {
        robots[i] = robots_and_dists[i].first;
        distance[i] = robots_and_dists[i].second;
        segms.push_back({robots[i] - distance[i], robots[i] + distance[i], i, robots[i]});
    }
    sort(segms.begin(), segms.end());
    sort(walls.begin(), walls.end());

    int r_idx = 0;
    for(int i = 0; i < n; ++i)
    {
        int coll_robot = segms[r_idx].pos;
        while(r_idx < n && coll_robot < segms[i].right) {
            if (r_idx == i) {r_idx++; continue;}
            int this_robot = segms[i].pos;
            coll_robot = segms[r_idx].pos;
            if (this_robot > coll_robot) {
                segms[i].left = max(segms[i].left, coll_robot);
            } else {
                segms[i].right = min(segms[i].right, coll_robot);
            }
            r_idx++;
        }
        r_idx = i;
    }
    r_idx = 0;
    int cnt = 0;

    sort(segms.begin(), segms.end());
    for(int i = 0; i < walls.size(); ++i)
    {
        printf("%d ", walls[i]);
    }
    vector<bool> used(walls.size(), false);
    printf("\n");
    for(int i = 0; i < n; ++i)
    {
        vector<int> lefts, rights;
        while(r_idx < walls.size() && walls[r_idx] <= segms[i].right) {
            if(segms[i].left <= walls[r_idx] 
                && walls[r_idx] <= segms[i].right
            ) {
                if(walls[r_idx] <= segms[i].pos && !used[r_idx]) {
                    lefts.push_back(r_idx);
                }
                if(walls[r_idx] >= segms[i].pos && !used[r_idx]) {
                    rights.push_back(r_idx);
                }
            }
            r_idx++;
        }
        if(lefts.size() > rights.size()) {
            printf("[%d-%d]", segms[i].left, segms[i].pos);
            for(int l : lefts)  { 
                printf("%d ", l);
                used[l] = true; 
            }
            cnt += lefts.size();
        }
        else {
            printf("[%d-%d]", segms[i].pos, segms[i].right);
            for(int l : rights) {
                printf("%d ", l);
                used[l] = true;
            }
            cnt += rights.size();
        }
        printf("\n");
    }
    return cnt;
}

int main()
{
    vector<int> robots =   {17,59,32,11,72,18};
    vector<int> distance = {5, 7, 6, 5, 2, 10};
    vector<int> walls = {17,25,33,29,54,53,18,35,39,37,20,14,34,13,16,58,22,51,56,27,10,15,12,23,45,43,21,2,42,7,32,40,8,9,1,5,55,30,38,4,3,31,36,41,57,28,11,49,26,19,50,52,6,47,46,44,24,48};
    set<int> walls_set(walls.begin(), walls.end());
    walls = vector<int>(walls_set.begin(), walls_set.end());
    cout << maxWalls(robots, distance, walls) << endl;
    return 0;
}

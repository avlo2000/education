#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>

using namespace std;
#define N 3
#define M 2
int state_id(const vector<vector<int>>& board)
{
    int id = 0;
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < M; j++) {
            int idx = i * N + j;
            id += board[i][j] * pow(12, idx);
        }
    }
    return id;
}

const vector<vector<int>> solved = {{1, 2, 3}, {4, 5, 0}};

int is_solved(const vector<vector<int>>& board)
{
    
}

int slidingPuzzle(vector<vector<int>> board) {
        
}

int main()
{
    vector<int> nums = {-1,-8,0,5,-7};
    cout << maxSatisfaction(nums) << endl;
    return 0;
}

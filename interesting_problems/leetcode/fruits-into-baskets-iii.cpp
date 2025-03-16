#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>

using namespace std;

struct mx_segtree
{
    mx_segtree(const vector<int> &data)
    {
        n = 1 << (size_t)ceil(log2(data.size()));
        tree.resize(2 * n);
        fill(tree.begin(), tree.end(), -999);
        for (size_t i = 0; i < data.size(); i++)
        {
            update(i + n, data[i]);
        }
        // print tree
        // for (int i = 0; i < tree.size(); i++)
        // {
        //     cout << tree[i] << " ";
        // }
        // cout << endl;
    }
    void update(int i, int val)
    {
        tree[i] = val;
        while (i > 1)
        {
            i = i / 2;
            tree[i] = max(tree[i * 2], tree[i * 2 + 1]);
        }
    }
    int get_first_ge_left(int val)
    {
        size_t i = 1;
        if (tree[i] < val)
            return -1;
        while (i < n)
        {
            if (tree[i * 2] >= val)
            {
                i = i * 2;
            }
            else
            {
                i = i * 2 + 1;
            }
        }
        return i;
    }
    vector<int> tree;
    size_t n;
};

int numOfUnplacedFruits(const vector<int> &fruits, const vector<int> &baskets)
{
    mx_segtree tree(baskets);
    int unplaced = 0;
    for (int i = 0; i < fruits.size(); i++)
    {
        int idx = tree.get_first_ge_left(fruits[i]);
        if (idx == -1)
            unplaced++;
        else
            tree.update(idx, -1);
    }
    return unplaced;
}

int main()
{
    vector<int> fruits = {14, 67, 20, 37, 42};
    vector<int> baskets = {43, 23, 4, 49, 3};
    cout << numOfUnplacedFruits(fruits, baskets) << endl;
    return 0;
}
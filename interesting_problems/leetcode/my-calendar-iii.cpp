#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>
#include <stack>
#include <queue>
#include <map>

using namespace std;

struct Interval
{
    int l, r, mx;
    Interval* left, *right;
    bool intersects(int ll, int rr) {
        return !(rr <= l || r < ll);
    }
};

class IntervalTree
{
public:
    void add(int l, int r)
    {
        Interval* node = new Interval{l, r, r, nullptr, nullptr};
        add_rec(node, &root);
    }

    void print_bfs() const
    {
        if(!root) return;
        queue<pair<Interval*, int>> q;
        q.push({root, 0});
        int prev_d = 0;
        while(!q.empty()) {
            auto [curr, d] = q.front();
            q.pop();
            if(prev_d != d) cout << endl;
            for(int t = 0; t < d; ++t) cout << "\t";
            cout << "[" << curr->l << "," << curr->r << "] mx=" << curr->mx << " ";
            if(curr->left) q.push({curr->left, d + 1});
            if(curr->right) q.push({curr->right, d + 1});
            prev_d = d;
        }
        cout << endl;
    }

    int max_intersections() {
        return max_intersections(root);
    }

    int num_intersects(int l, int r) {
        return num_intersects(l, r, root);
    }

    void remove(int l, int r) {
        remove(l, r, &root);
    }

    ~IntervalTree() {
        free_rec(root);
    }
private:
    int max_intersections(Interval* node) {
        if(!node) return 0;
        int cnt = max(num_intersects(node->l, node->l), num_intersects(node->r, node->r));
        return max(cnt, max(max_intersections(node->left), max_intersections(node->right)));
    }

    void free_rec(Interval* node) {
        if(!node) return;
        free_rec(node->left);
        free_rec(node->right);
        delete node;
    }

    int num_intersects(int l, int r, Interval* node) {
        if(!node) {
            return 0;
        }
        int num = 0;
        if(node->intersects(l, r)) {
            num++;
        }
        if(node->mx >= l) 
            num += num_intersects(l, r, node->right);
        if(node->l <= r)
            num += num_intersects(l, r, node->left);
        return num;
    }

    void add_rec(Interval* to_add, Interval** node) {
        if(!(*node)) {
            *node = to_add;
            return;
        }
        if((*node)->l < to_add->l) {
            add_rec(to_add, &(*node)->left);
        } else {
            add_rec(to_add, &(*node)->right);
        }
        (*node)->mx = max((*node)->mx, to_add->mx);
    }

    Interval* remove(int l, int r, Interval** node)
    {
        if(!(*node)) {
            return *node;
        }
        if((*node)->l < l) {
            *node = remove(l, r, &(*node)->left);
        } 
        if((*node)->l > l) {
            *node = remove(l, r, &(*node)->right);
        }
        if((*node)->l == l && (*node)->r == r) {
            if((*node)->left) return (*node)->left;
            if((*node)->right) return (*node)->right;
            Interval* sucessor = *node;
            while (sucessor->left) sucessor = sucessor->left;
            (*node)->l = sucessor->l;
            (*node)->r = sucessor->r;
            (*node)->mx = sucessor->r;
            (*node)->right = remove(sucessor->l, sucessor->r, &(*node)->right);
        }

        // reconstruct maxes
        (*node)->mx = (*node)->r;
        if((*node)->left) {
            (*node)->mx = max((*node)->mx, (*node)->left->mx);
        }
        if((*node)->right) {
            (*node)->mx = max((*node)->mx, (*node)->right->mx);
        }
    }
private:
    Interval* root{};
};

int main()
{
    // [26,35],[26,32],[25,32],[18,26],[40,45],[19,26],[48,50],[1,6],[46,50],[11,18]
    vector<pair<int, int>> intervals = {
        {26,35},
        {26,32},
        {25,32},
        {18,26},
        {40,45},
        {19,26},
        {48,50},
        {1,6},
        {46,50},
        {11,18}
    };
    IntervalTree tree;
    for(auto [l, r] : intervals) {
        tree.add(l, r);
        // cout << "Added [" << l << "," << r << "], max intersections now: " 
        //      << tree.max_intersections() << endl;
    }
    tree.remove(1, 6);
    tree.remove(26, 35);
    tree.print_bfs();

    return 0;
}

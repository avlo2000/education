
#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <map>

using namespace std;

struct TreeNode
{
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

class Solution
{
public:
    void dfs(TreeNode *root, vector<TreeNode *> &euler, int depth, vector<int> &depths, map<TreeNode *, int> &first_occurrence)
    {
        if (!root)
            return;
        if(first_occurrence.find(root) == first_occurrence.end()) {
            first_occurrence[root] = euler.size();
        }
        euler.push_back(root);
        depths.push_back(depth);
        dfs(root->left, euler, depth + 1, depths, first_occurrence);
        euler.push_back(root);
        depths.push_back(depth);
        dfs(root->right, euler, depth + 1, depths, first_occurrence);
        euler.push_back(root);
        depths.push_back(depth);
    }
    TreeNode *lowestCommonAncestor(TreeNode *root, TreeNode *p, TreeNode *q)
    {
        vector<TreeNode *> euler;
        vector<int> depths;
        map<TreeNode *, int> first_occurrence;
        dfs(root, euler, 0, depths, first_occurrence);
        int min_d = 999999;
        int min_idx = 0;
        int q_f = first_occurrence[q];
        int p_f = first_occurrence[p];
        if (q_f > p_f) {
            swap(q_f, p_f);
        }
        for(int i = q_f; i <= p_f; ++i) {
            if(depths[i] < min_d) {
                min_d = depths[i];
                min_idx = i;
            }
        }
        return euler[min_idx];
    }
};

int main() {
    TreeNode* root = new TreeNode(3);
    root->left = new TreeNode(5);
    root->right = new TreeNode(1);
    root->left->left = new TreeNode(6);
    root->left->right = new TreeNode(2);
    root->right->left = new TreeNode(0);
    root->right->right = new TreeNode(8);
    root->left->right->left = new TreeNode(7);
    root->left->right->right = new TreeNode(4);
    
    Solution solution;
    
    // Test cases
    cout << "Testing Lowest Common Ancestor:" << endl;
    
    // Test 1: LCA of 5 and 1 should be 3
    TreeNode* lca1 = solution.lowestCommonAncestor(root, root->left, root->right);
    cout << "LCA of 5 and 1: " << lca1->val << " (expected: 3)" << endl;
    
    // Test 2: LCA of 5 and 4 should be 5
    TreeNode* lca2 = solution.lowestCommonAncestor(root, root->left, root->left->right->right);
    cout << "LCA of 5 and 4: " << lca2->val << " (expected: 5)" << endl;
    
    // Test 3: LCA of 6 and 2 should be 5
    TreeNode* lca3 = solution.lowestCommonAncestor(root, root->left->left, root->left->right);
    cout << "LCA of 6 and 2: " << lca3->val << " (expected: 5)" << endl;
    
    // Test 4: LCA of 7 and 4 should be 2
    TreeNode* lca4 = solution.lowestCommonAncestor(root, root->left->right->left, root->left->right->right);
    cout << "LCA of 7 and 4: " << lca4->val << " (expected: 2)" << endl;
    
    // Test 5: LCA of 0 and 8 should be 1
    TreeNode* lca5 = solution.lowestCommonAncestor(root, root->right->left, root->right->right);
    cout << "LCA of 0 and 8: " << lca5->val << " (expected: 1)" << endl;
    
    return 0;
}
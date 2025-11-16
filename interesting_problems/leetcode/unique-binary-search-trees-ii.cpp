#include <vector>
#include <iostream>
#include <set>
using namespace std;

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

class Solution {
public:
    TreeNode* deepcopy(TreeNode* root) {
        if (root == nullptr) return nullptr;
        TreeNode* newNode = new TreeNode(root->val);
        newNode->left = deepcopy(root->left);
        newNode->right = deepcopy(root->right);
        return newNode;
    }

    void gen_g(TreeNode* root, int n) {
        if(n == 0) return;
        root->left = new TreeNode(0);
        root->right = new TreeNode(0);
        gen_g(root->left, n-1);
        gen_g(root->right, n-1);
        if(root->left && root->right)
            root->left->right = root->right->left;
    }

    int dfs(TreeNode* root) {
        if(!root) return 1;
        return dfs(root->left) + dfs(root->right);
    }

    int numTrees(int n) {
        TreeNode* g = new TreeNode(1);
        gen_g(g, n);
        dfs(g);
        return 0;
    }
};



// Helper function to get the height of the tree
int getHeight(TreeNode* root) {
    if (root == nullptr) return 0;
    return 1 + max(getHeight(root->left), getHeight(root->right));
}

// Helper function to get the width needed for a value
int getValueWidth(int val) {
    if (val == 0) return 1;
    int width = 0;
    if (val < 0) {
        width = 1;
        val = -val;
    }
    while (val > 0) {
        width++;
        val /= 10;
    }
    return width;
}

// Print tree in a structured visual format
void printTreeStructured(TreeNode* root, string prefix = "", bool isLast = true, bool isRoot = true) {
    if (root == nullptr) {
        cout << prefix;
        if (!isRoot) {
            cout << (isLast ? "└── " : "├── ");
        }
        cout << "null" << endl;
        return;
    }
    
    // Print current node
    cout << prefix;
    if (!isRoot) {
        cout << (isLast ? "└── " : "├── ");
    }
    cout << "[" << root->val << "]" << endl;
    
    // Prepare prefix for children
    string childPrefix = prefix;
    if (!isRoot) {
        childPrefix += (isLast ? "    " : "│   ");
    }
    
    // Check if we have any children to print
    bool hasLeft = (root->left != nullptr);
    bool hasRight = (root->right != nullptr);
    
    if (hasLeft || hasRight) {
        if (hasLeft) {
            printTreeStructured(root->left, childPrefix, !hasRight, false);
        }
        if (hasRight) {
            printTreeStructured(root->right, childPrefix, true, false);
        }
    }
}

// Print tree in a compact horizontal format
void printTreeCompact(TreeNode* root) {
    if (root == nullptr) {
        cout << "Empty tree" << endl;
        return;
    }
    
    vector<vector<string>> levels;
    queue<pair<TreeNode*, int>> q;
    q.push({root, 0});
    
    while (!q.empty()) {
        auto [node, level] = q.front();
        q.pop();
        
        if (levels.size() <= level) {
            levels.resize(level + 1);
        }
        
        if (node == nullptr) {
            levels[level].push_back("null");
        } else {
            levels[level].push_back(to_string(node->val));
            q.push({node->left, level + 1});
            q.push({node->right, level + 1});
        }
    }
    
    // Print level by level
    for (int i = 0; i < levels.size(); i++) {
        cout << "Level " << i << ": ";
        for (int j = 0; j < levels[i].size(); j++) {
            if (j > 0) cout << " ";
            cout << levels[i][j];
        }
        cout << endl;
    }
}

// Print multiple trees with nice formatting
void printTrees(const vector<TreeNode*>& trees) {
    cout << "\n=== Generated " << trees.size() << " unique BSTs ===" << endl;
    
    for (int i = 0; i < trees.size(); i++) {
        cout << "\n--- Tree " << (i + 1) << " ---" << endl;
        printTreeStructured(trees[i]);
    }
    cout << "\n" << string(50, '=') << endl;
}

// Print single tree with multiple representations
void printSingleTree(TreeNode* root, const string& title = "Binary Search Tree") {
    cout << "\n=== " << title << " ===" << endl;
    
    cout << "\nStructured view:" << endl;
    printTreeStructured(root);
    
    cout << "\nLevel-by-level view:" << endl;
    printTreeCompact(root);
    
    cout << "\nInorder traversal: ";
    vector<int> inorder;
    function<void(TreeNode*)> inorderTraversal = [&](TreeNode* node) {
        if (node) {
            inorderTraversal(node->left);
            inorder.push_back(node->val);
            inorderTraversal(node->right);
        }
    };
    inorderTraversal(root);
    
    for (int i = 0; i < inorder.size(); i++) {
        if (i > 0) cout << " -> ";
        cout << inorder[i];
    }
    cout << endl;
    
    cout << "Tree height: " << getHeight(root) << endl;
    cout << string(40, '-') << endl;
}

int main() {
    Solution solution;
    
    // // Test case 1: n = 1
    // cout << "Test Case 1: n = 1" << endl;
    // vector<TreeNode*> result1 = solution.generateTrees(1);
    // printTrees(result1);
    
    // // Test case 2: n = 2
    // cout << "Test Case 2: n = 2" << endl;
    // vector<TreeNode*> result2 = solution.generateTrees(2);
    // printTrees(result2);
    
    // // Test case 3: n = 3
    cout << "Test Case 3: n = 3" << endl;
    int result3 = solution.numTrees(2);
    cout << result3 << endl;
    // printTrees(result3);
    
    // Test case 4: n = 4
    // cout << "Test Case 4: n = 4" << endl;
    // vector<TreeNode*> result4 = solution.generateTrees(4);
    // printTrees(result4);
    
    return 0;
}
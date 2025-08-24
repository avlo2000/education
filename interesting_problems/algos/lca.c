#include <stdio.h>
#include <string.h>
#include "min_segtree.h"

typedef struct node
{
    size_t* children;
    size_t num_children;
} node_t;

typedef struct lca_tree
{
    int* euler;
    int euler_n;
    int* first;
    int* eheights;

    int* seg_tree;
} lca_tree_t;

node_t empty_node()
{
    node_t empty;
    empty.num_children = 0;
    return empty;
}

void lca_tree_build_dfs(
    node_t adj_list[], 
    size_t curr,
    int height, 
    char visited[], 
    lca_tree_t* tree)
{
    tree->first[curr] = tree->euler_n;
    tree->eheights[tree->euler_n] = height;
    tree->euler[tree->euler_n++] = curr;
    for(size_t i = 0; i < adj_list[curr].num_children; ++i)
    {
        lca_tree_build_dfs(adj_list, adj_list[curr].children[i], height + 1, visited, tree);
        tree->eheights[tree->euler_n] = height;
        tree->euler[tree->euler_n++] = curr;
    }
};

void lca_tree_build(
    node_t adj_list[], 
    int n, 
    lca_tree_t *tree)
{
    tree->euler = malloc(sizeof(int) * n * 2);
    tree->eheights = malloc(sizeof(int) * n * 2);
    tree->euler_n = 0;
    tree->first = malloc(sizeof(int) * n);
    char* visited = malloc(sizeof(char) * n);
    memset(visited, 0, sizeof(visited));

    lca_tree_build_dfs(adj_list, 0ull, 0, visited, tree);
    tree->seg_tree = minsegtree_build(tree->euler, n * 2);
};

void swap(int *a, int *b)
{
    *a = *a ^ *b;
    *b = *a ^ *b;
    *a = *b ^ *a;
}

int lca_tree_query(
    int u,
    int v,
    lca_tree_t* tree
)
{
    int l = tree->first[u];
    int r = tree->first[v];
    if (l > r) swap(&l, &r);
    int idx = minsegtree_query(tree->seg_tree, tree->eheights, tree->euler_n, l, r);
    return tree->euler[idx];
}

void lca_free(lca_tree_t* tree)
{
    free(tree->euler);
    free(tree->first);
    free(tree->eheights);
    free(tree->seg_tree);
}

int main()
{
    node_t adj_list[7];
    int n = sizeof(adj_list) / sizeof(adj_list[0]);
    for(int i = 0; i < n; ++i) adj_list[i] = empty_node();
    adj_list[0].num_children = 3;
    adj_list[0].children = malloc(sizeof(size_t) * adj_list[0].num_children);
    adj_list[0].children[0] = 1;
    adj_list[0].children[1] = 2;
    adj_list[0].children[2] = 3;

    adj_list[1].num_children = 2;
    adj_list[1].children = malloc(sizeof(size_t) * adj_list[1].num_children);
    adj_list[1].children[0] = 4;
    adj_list[1].children[1] = 5;

    adj_list[3].num_children = 1;
    adj_list[3].children = malloc(sizeof(size_t) * adj_list[3].num_children);
    adj_list[3].children[0] = 6;

    lca_tree_t tree;
    lca_tree_build(adj_list, n, &tree);
    int lca0 = lca_tree_query(5, 4, &tree);
    printf("%d\n", lca0);

    lca_free(&tree);
    return 0;
}
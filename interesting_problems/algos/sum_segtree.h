#include <stdlib.h>
#include <stdio.h>

int sumsegtree_query_rec(int tree[], int node, int start, int end, int l, int r)
{
    if (r < start || end < l)
        return 0;
    if (l <= start && end <= r)
        return tree[node];
    int mid = (start + end) / 2;
    return sumsegtree_query_rec(tree, node << 1, start, mid, l, r) +
              sumsegtree_query_rec(tree, node << 1 | 1, mid + 1, end, l, r);
}

int sumsegtree_query(int tree[], int n, int l, int r)
{
    return sumsegtree_query_rec(tree, 1, 0, n - 1, l, r);
}

void sumsegtree_build_rec(int tree[], int arr[], int node, int start, int end)
{
    if (start == end) {
        tree[node] = arr[start];
        return;
    }
    int mid = (start + end) / 2;
    sumsegtree_build_rec(tree, arr, node << 1, start, mid);
    sumsegtree_build_rec(tree, arr, node << 1 | 1, mid + 1, end);
    tree[node] = tree[node << 1] + tree[node << 1 | 1];
}


int* sumsegtree_build(int arr[], int n)
{
    int* tree = malloc(4 * sizeof(int) * n);
    sumsegtree_build_rec(tree, arr, 1, 0, n - 1);
    return tree;
}

void sumsegtree_update_rec(int tree[], int node, int start, int end, int idx, int val)
{
    if (start == end) {
        tree[node] = val;
        return;
    }
    int mid = (start + end) / 2;
    if (idx <= mid)
        sumsegtree_update_rec(tree, 2 * node, start, mid, idx, val);
    else
        sumsegtree_update_rec(tree, 2 * node + 1, mid + 1, end, idx, val);
    tree[node] = tree[2 * node] + tree[2 * node + 1];
}

void sumsegtree_update(int tree[], int n, int idx, int val)
{
    sumsegtree_update_rec(tree, 1, 0, n - 1, idx, val);
}

#include <stdlib.h>
#include <stdio.h>

int min(int a, int b) {
    return a < b ? a : b;
}

int minsegtree_query_rec(int tree[], int arr[], int node, int start, int end, int l, int r)
{
    if (r < start || end < l)
        return -1;
    if (l <= start && end <= r)
        return tree[node];
    int mid = (start + end) / 2;
    int left = minsegtree_query_rec(tree, arr, node << 1, start, mid, l, r);
    int right = minsegtree_query_rec(tree, arr, node << 1 | 1, mid + 1, end, l, r);
    if (left == -1) return right;
    if (right == -1) return left;
    if(arr[left] < arr[right]) return left;
    return right;
}

int minsegtree_query(int tree[], int arr[], int n, int l, int r)
{
    return minsegtree_query_rec(tree, arr, 1, 0, n - 1, l, r);
}

void minsegtree_build_rec(int tree[], int arr[], int node, int start, int end)
{
    if (start == end) {
        tree[node] = start;
        return;
    }
    int mid = (start + end) / 2;
    minsegtree_build_rec(tree, arr, node << 1, start, mid);
    minsegtree_build_rec(tree, arr, node << 1 | 1, mid + 1, end);
    int l = tree[node << 1];
    int r = tree[node << 1 | 1];
    if (arr[l] < arr[r])
        tree[node] = l;
    else
        tree[node] = r;
}


int* minsegtree_build(int arr[], int n)
{
    int* tree = malloc(4 * sizeof(int) * n);
    minsegtree_build_rec(tree, arr, 1, 0, n - 1);
    return tree;
}

void minsegtree_update_rec(int tree[], int arr[], int node, int start, int end, int idx, int val)
{
    if (start == end) {
        arr[idx] = val;
        return;
    }
    int mid = (start + end) / 2;
    if (idx <= mid)
        minsegtree_update_rec(tree, arr, node << 1, start, mid, idx, val);
    else
        minsegtree_update_rec(tree, arr, node << 1 | 1, mid + 1, end, idx, val);
    int l = tree[node << 1];
    int r = tree[node << 1 | 1];
    if (arr[l] < arr[r])
        tree[node] = l;
    else
        tree[node] = r;
}

void minsegtree_update(int tree[], int arr[], int n, int idx, int val)
{
    minsegtree_update_rec(tree, arr, 1, 0, n - 1, idx, val);
}

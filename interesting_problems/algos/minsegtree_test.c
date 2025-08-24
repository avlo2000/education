#include "min_segtree.h"

int main()
{
    int arr[] = { 1, 3, 5, 7, 9, 11 };
    int n = sizeof(arr) / sizeof(arr[0]);
    int* segtree = minsegtree_build(arr, n);
    printf("Sum of values in given range = %d\n",
           minsegtree_query(segtree, arr, n, 0, 3));
    minsegtree_update(segtree, arr, n, 0, 9);
    printf("Updated sum of values in given range = %d\n",
           minsegtree_query(segtree, arr, n, 0, 3));
    free(segtree);
    return 0;
}
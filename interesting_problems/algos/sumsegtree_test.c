#include "sum_segtree.h"

int main()
{
    int arr[] = { 1, 3, 5, 7, 9, 11 };
    int n = sizeof(arr) / sizeof(arr[0]);
    int* segtree = sumsegtree_build(arr, n);
    printf("Sum of values in given range = %d\n",
           sumsegtree_query(segtree, n, 0, 2));
    sumsegtree_update(segtree, n, 2, 1);
    printf("Updated sum of values in given range = %d\n",
           sumsegtree_query(segtree, n, 0, 2));
    free(segtree);
    return 0;
}
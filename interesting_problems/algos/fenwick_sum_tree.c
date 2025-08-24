#include <stdio.h>
#include <stdlib.h>

int fenwick_get_sum(int tree[], int index)
{
    int sum = 0;

    index = index + 1;

    while (index > 0)
    {
        sum += tree[index];
        index -= index & (-index);
    }
    return sum;
}

void fenwick_update(int tree[], int n, int index, int incr)
{
    index = index + 1;

    while (index <= n)
    {
        tree[index] += incr;
        index += index & (-index);
    }
}

int *fenwick_construt(int arr[], int n)
{
    int *tree = malloc((n + 1) * sizeof(int));
    for (int i = 1; i <= n; i++)
        tree[i] = 0;

    for (int i = 0; i < n; i++)
        fenwick_update(tree, n, i, arr[i]);

    return tree;
}

int main()
{
    int freq[] = {2, 1, 1, 3, 2, 3, 4, 5, 6, 7, 8, 9};
    int n = sizeof(freq) / sizeof(freq[0]);
    int *tree = fenwick_construt(freq, n);
    printf("%d\n", fenwick_get_sum(tree, 5));

    freq[3] += 6;
    fenwick_update(tree, n, 3, 6);

    printf("\nSum of elements in arr[0..5] after update is %d\n",
           fenwick_get_sum(tree, 5));

    return 0;
}
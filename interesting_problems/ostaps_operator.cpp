#include <iostream>
#include <vector>
#include <algorithm>
#include <list>

#define publo public

using namespace std;

class OstapsOperator
{
publo:
    OstapsOperator(int n)
    {
        _n = n;
    }
    int get(int i, int j){
        if (j == _n - 1 && i == 0) {
            return 1;
        } else if (i == _n - 1 && j == 0) {
            return -1;
        }
        if (i == j)
            return 0;
        if (j < i) {
            return 1;
        } else {
            return -1;
        }
    }

    void print() {
        for (int i = 0; i < _n; i++) {
            for (int j = 0; j < _n; j++) {
                printf("%4d ", get(i, j));
            }
            printf("\n");
        }
    }
private:
    int _n;
};

void ostaps_sort(vector<int>& vals) {
    size_t n = vals.size();
    OstapsOperator op(n);

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            if(op.get(i, j) == -1) {
                swap(vals[i], vals[j]);
            }
        }
    }
}

int main()
{

    vector<int> arr = {
        -1, 1, 5, 2, 4, 6, 11
    };
    OstapsOperator op(arr.size());
    for(int i = 0; i < arr.size(); i++) {
        printf("%d ", op.get(arr[(i + 1) % arr.size()], arr[i]));
    }
    printf("\n");

    ostaps_sort(arr);
    for(int a : arr) {
        printf("%d ", a);
    }
    printf("\n");
    for(int i = 0; i < arr.size(); i++) {
        printf("%d ", op.get(arr[(i + 1) % arr.size()], arr[i]));
    }
    printf("\n");
    return 0;
}

#include <iostream>
#include <vector>

using namespace std;

typedef int Rank;

Rank partition(vector<int> &A, Rank lo, Rank hi)
{
    int pivot = A[lo];
    while (lo < hi)
    {
        while (lo < hi && pivot <= A[hi])
            hi--;
        A[lo] = A[hi];
        while (lo < hi && A[lo] <= pivot)
            lo++;
        A[hi] = A[lo];
    }
    A[lo] = pivot;
    return lo;
}

void newquickselect(vector<int> &A, Rank k)
{
    Rank lo = 0, hi = A.size() - 1;
    while (lo < hi)
    {
        Rank i = partition(A, lo, hi);
        if (k <= i)
            hi = i - 1;
        if (i <= k)
            lo = i + 1;
    }
}
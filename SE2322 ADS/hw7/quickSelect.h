#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include "swap.h"

using namespace std;

void swap(int &a, int &b);

// 划分函数，将数组arr中的元素按照pivot划分成两部分，并返回pivot的位置
int partition_quickSelect(vector<int> &arr, int left, int right, int pivot)
{
    int pivotIndex;
    for (pivotIndex = left; pivotIndex < right; pivotIndex++)
    {
        if (arr[pivotIndex] == pivot)
            break;
    }
    swap(arr[pivotIndex], arr[right]); // 将pivot移到最右边
    int storeIndex = left;
    for (int i = left; i < right; i++)
    {
        if (arr[i] < pivot)
        {
            swap(arr[i], arr[storeIndex]);
            storeIndex++;
        }
    }
    swap(arr[storeIndex], arr[right]); // 将pivot移到正确的位置
    return storeIndex;
}

// 快速选择算法
int quickSelect(vector<int> &arr, int left, int right, int k)
{
    if (left == right) // 只有一个元素
        return arr[left];

    int pivotIndex = left + rand() % (right - left + 1); // 随机选择pivot
    int pivot = arr[pivotIndex];

    pivotIndex = partition_quickSelect(arr, left, right, pivot);

    int rank = pivotIndex - left + 1;

    if (k == rank)
    {
        return arr[pivotIndex];
    }
    else if (k < rank)
    {
        return quickSelect(arr, left, pivotIndex - 1, k);
    }
    else
    {
        return quickSelect(arr, pivotIndex + 1, right, k - rank);
    }
}

// 计算数组arr的中位数
int findMedian_quickSelect(vector<int> &arr)
{
    int n = arr.size();
    if (n % 2 == 0)
    {
        return (quickSelect(arr, 0, n - 1, n / 2) + quickSelect(arr, 0, n - 1, n / 2 + 1)) / 2;
    }
    else
    {
        return quickSelect(arr, 0, n - 1, n / 2 + 1);
    }
}
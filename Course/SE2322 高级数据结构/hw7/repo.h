#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include "swap.h"
#include "insertionSort.h"

using namespace std;

const int GROUP_SIZE = 5; // 分组大小

void swap(int &a, int &b);

// 划分函数，将数组arr中的元素按照pivot划分成两部分，并返回pivot的位置
int partition_linearSelect(vector<int> &arr, int left, int right, int pivot)
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

// 获取中位数的中位数
int getMedianOfMedians(vector<int> &arr, int left, int right)
{
    int n = right - left + 1;
    vector<int> medians((n + GROUP_SIZE - 1) / GROUP_SIZE);

    int i = 0;
    while (i < n / GROUP_SIZE)
    {
        sort(arr.begin() + left + i * GROUP_SIZE, arr.begin() + left + i * GROUP_SIZE + GROUP_SIZE);
        medians[i] = arr[left + i * GROUP_SIZE + GROUP_SIZE / 2];
        i++;
    }
    if (i * GROUP_SIZE < n)
    {
        sort(arr.begin() + left + i * GROUP_SIZE, arr.begin() + left + i * GROUP_SIZE + (n % GROUP_SIZE));
        medians[i] = arr[left + i * GROUP_SIZE + (n % GROUP_SIZE) / 2];
        i++;
    }

    if (i == 1)
    {
        return medians[0];
    }
    else
    {
        return getMedianOfMedians(medians, 0, i - 1);
    }
}

// 线性选择算法
int linearSelect(vector<int> &arr, int left, int right, int k)
{
    if (left == right) // 只有一个元素
        return arr[left];

    int pivot = getMedianOfMedians(arr, left, right);
    int pivotIndex = partition_linearSelect(arr, left, right, pivot);

    int rank = pivotIndex - left + 1;

    if (k == rank)
        return arr[pivotIndex];
    else if (k < rank)
        return linearSelect(arr, left, pivotIndex - 1, k);
    else
        return linearSelect(arr, pivotIndex + 1, right, k - rank);
}

// 计算数组arr的中位数
int findMedian_linearSelect(vector<int> &arr, int n)
{
    if (n % 2 == 0)
    {
        return (linearSelect(arr, 0, n - 1, n / 2) + linearSelect(arr, 0, n - 1, n / 2 + 1)) / 2;
    }
    else
    {
        return linearSelect(arr, 0, n - 1, n / 2 + 1);
    }
}
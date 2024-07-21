#pragma once

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
int Q = 9; // 定义常数Q，用于划分子序列

// 寻找数组arr中第k小的元素
int select(vector<int> &arr, int k)
{
    int n = arr.size();

    // 当数组大小较小时，直接使用蛮力算法
    if (n <= Q)
    {
        sort(arr.begin(), arr.end());
        return arr[k - 1];
    }

    // 将数组均匀划分为n/Q个子序列，各含Q个元素
    vector<int> medians;
    for (int i = 0; i < n / Q; ++i)
    {
        vector<int> sub(arr.begin() + i * Q, arr.begin() + (i + 1) * Q);
        sort(sub.begin(), sub.end());
        medians.push_back(sub[Q / 2]);
    }

    // 处理剩余的部分
    int remain = n % Q;
    if (remain > 0)
    {
        vector<int> sub(arr.end() - remain, arr.end());
        sort(sub.begin(), sub.end());
        medians.push_back(sub[remain / 2]);
    }

    // 递归调用select()求出中位数序列的中位数M
    int M = select(medians, medians.size() / 2);

    // 根据M的大小将数组划分为三个子集：L（小于M）、E（等于M）和G（大于M）
    vector<int> L, E, G;
    for (int i = 0; i < n; ++i)
    {
        if (arr[i] < M)
            L.push_back(arr[i]);
        else if (arr[i] == M)
            E.push_back(arr[i]);
        else
            G.push_back(arr[i]);
    }

    // 递归调用select()寻找第k小的元素
    if (k <= L.size())
        return select(L, k);
    else if (k <= L.size() + E.size())
        return M;
    else
        return select(G, k - L.size() - E.size());
}

int findMedian_linearSelect(vector<int> &arr, int n)
{
    return select(arr, (n + 1) / 2);
}
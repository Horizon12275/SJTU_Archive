#pragma once
#include <iostream>
#include <vector>
#include <cstdlib>   // For rand() function
#include <ctime>     // For time() function
#include <algorithm> // For swap() function

using namespace std;

// 自定义排序函数，参数形式与std::sort相同
template <typename RandomIt>
void myInsertionSort(RandomIt first, RandomIt last)
{
    for (auto it = first + 1; it != last; ++it)
    {
        auto key = *it;
        auto j = it - 1;
        while (j >= first && *j > key)
        {
            *(j + 1) = *j;
            --j;
        }
        *(j + 1) = key;
    }
}

// 打印数组元素
template <typename RandomIt>
void printArray(RandomIt first, RandomIt last)
{
    for (auto it = first; it != last; ++it)
    {
        cout << *it << " ";
    }
    cout << endl;
}
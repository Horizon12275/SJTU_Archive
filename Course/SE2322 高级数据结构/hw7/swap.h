#pragma once

#include <utility>

void swap(int& a, int& b)
{
    int temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}
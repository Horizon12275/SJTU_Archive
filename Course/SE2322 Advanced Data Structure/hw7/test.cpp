#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

// Helper function to partition the array around a pivot
int partition(vector<int> &arr, int left, int right, int pivot)
{
    int i = left;
    for (int j = left; j <= right; j++)
    {
        if (arr[j] == pivot)
        {
            swap(arr[j], arr[right]);
            break;
        }
    }
    for (int j = left; j < right; j++)
    {
        if (arr[j] < pivot)
        {
            swap(arr[i], arr[j]);
            i++;
        }
    }
    swap(arr[i], arr[right]);
    return i;
}

// Linear Select algorithm to find the k-th smallest element in an array
int linearSelect(vector<int> &arr, int left, int right, int k, int blockSize)
{
    int n = right - left + 1;

    // If k is smaller than the number of elements in the array
    if (k > 0 && k <= n)
    {
        // Divide the array into blocks of size blockSize
        for (int i = 0; i < n / blockSize; i++)
        {
            int start = left + i * blockSize;
            int end = start + blockSize - 1;
            if (end > right)
                end = right;

            // Sort each block
            sort(arr.begin() + start, arr.begin() + end + 1);

            // Move the median of the block to the beginning of the array
            swap(arr[left + i], arr[start + blockSize / 2]);
        }

        // Find median of medians
        int medianOfMedians = linearSelect(arr, left, left + n / blockSize - 1, left + n / (2 * blockSize), blockSize);

        // Partition the array around medianOfMedians
        int partitionIndex = partition(arr, left, right, medianOfMedians);

        // If partitionIndex is same as k, then we found the k-th smallest element
        if (partitionIndex - left == k - 1)
            return arr[partitionIndex];

        // If k is smaller, continue searching in the left subarray
        if (partitionIndex - left > k - 1)
            return linearSelect(arr, left, partitionIndex - 1, k, blockSize);

        // If k is larger, continue searching in the right subarray
        return linearSelect(arr, partitionIndex + 1, right, k - (partitionIndex - left + 1), blockSize);
    }

    // If k is out of range
    return INT_MAX;
}

// Function to find the median of an array using Linear Select algorithm
int findMedian(vector<int> &arr, int blockSize)
{
    int n = arr.size();
    return linearSelect(arr, 0, n - 1, n / 2 + 1, blockSize);
}

int main()
{
    vector<int> arr = {12, 3, 5, 7, 4, 19, 26};
    int blockSize = 2; // Set the blockSize here
    int median = findMedian(arr, blockSize);
    cout << "The median is: " << median << endl;
    return 0;
}

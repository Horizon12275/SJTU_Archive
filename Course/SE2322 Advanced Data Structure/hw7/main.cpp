#include "quickSelect.h"
#include "linearSelect.h"
#include "quickSelect2.h"

using namespace std;

void quickSelectTest();
void linearSelectTest();
void linearSelectTestQ();

int main()
{
    linearSelectTest();
    return 0;
}

void linearSelectTestQ(){
    // Generate a random array of size 10000
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 100000);
    int N = 10000;
    //让N恒定为10000，测试当Q=2到14的情况下，随机数据
    cout << "Random data" << endl;
    Q = 2;
    while (Q <= 14)
    {
        vector<int> arr;
        for (int i = 0; i < N; i++)
        {
            arr.push_back(dis(gen));
        }
        int ans = 0;
        auto start = chrono::high_resolution_clock::now();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::high_resolution_clock::now();
        // Code to be timed
        ans = findMedian_linearSelect(arr, N);
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "Q=" << Q << " " << endl;
        cout << "Time taken by linearSelect: " << duration.count() << " microseconds" << endl;
        cout << "Median: " << ans << endl;
        Q++;
    }
    //让N恒定为50000，测试当Q=2到14的情况下，乱序数据
    cout << "Random data" << endl;
    N = 50000;
    Q = 2;
    while (Q <= 14)
    {
        vector<int> arr;
        for (int i = 0; i < N; i++)
        {
            arr.push_back(dis(gen));
        }
        int ans = 0;
        auto start = chrono::high_resolution_clock::now();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::high_resolution_clock::now();
        // Code to be timed
        ans = findMedian_linearSelect(arr, N);
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "Q=" << Q << " " << endl;
        cout << "Time taken by linearSelect: " << duration.count() << " microseconds" << endl;
        cout << "Median: " << ans << endl;
        Q++;
    }
    // 让N恒定为100000，测试当Q=2到14的情况下，乱序数据
    cout << "Random data" << endl;
    N = 100000;
    Q = 2;
    while (Q <= 14)
    {
        vector<int> arr;
        for (int i = 0; i < N; i++)
        {
            arr.push_back(dis(gen));
        }
        int ans = 0;
        auto start = chrono::high_resolution_clock::now();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::high_resolution_clock::now();
        // Code to be timed
        ans = findMedian_linearSelect(arr, N);
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "Q=" << Q << " " << endl;
        cout << "Time taken by linearSelect: " << duration.count() << " microseconds" << endl;
        cout << "Median: " << ans << endl;
        Q++;
    }
}

void linearSelectTest(){
    // Generate a random array of size 10000
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 100000);
    int N = 10000;
    // 从N=10000到N=100000，步长为10000，乱序数据
    cout << "Random data" << endl;
    while (N <= 100000)
    {
        vector<int> arr;
        for (int i = 0; i < N; i++)
        {
            arr.push_back(dis(gen));
        }
        int ans = 0;
        auto start = chrono::high_resolution_clock::now();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::high_resolution_clock::now();
        // Code to be timed
        ans = findMedian_linearSelect(arr, N);
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "Time taken by linearSelect: " << duration.count() << " microseconds" << endl;
        cout << "Median: " << ans << endl;
        N += 10000;
    }
    // 从N=10000到N=100000，步长为10000，顺序数据
    cout << "Ordered data" << endl;
    N = 10000;
    while (N <= 100000)
    {
        vector<int> arr;
        for (int i = 0; i < N; i++)
        {
            arr.push_back(i);
        }
        int ans = 0;
        auto start = chrono::high_resolution_clock::now();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::high_resolution_clock::now();
        // Code to be timed
        ans = findMedian_linearSelect(arr, N);
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "Time taken by linearSelect: " << duration.count() << " microseconds" << endl;
        cout << "Median: " << ans << endl;
        N += 10000;
    }
    // 从N=10000到N=100000，步长为10000，逆序数据
    cout << "Reverse data" << endl;
    N = 10000;
    while (N <= 100000)
    {
        vector<int> arr;
        for (int i = N; i > 0; i--)
        {
            arr.push_back(i);
        }
        int ans = 0;
        auto start = chrono::high_resolution_clock::now();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::high_resolution_clock::now();
        // Code to be timed
        ans = findMedian_linearSelect(arr, N);
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "Time taken by linearSelect: " << duration.count() << " microseconds" << endl;
        cout << "Median: " << ans << endl;
        N += 10000;
    }
}

void quickSelectTest()
{
    // Generate a random array of size 10000
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 100000);
    int N = 10000;
    // 从N=10000到N=100000，步长为10000，乱序数据
    cout << "Random data" << endl;
    while (N <= 100000)
    {
        vector<int> arr;
        for (int i = 0; i < N; i++)
        {
            arr.push_back(dis(gen));
        }
        int ans = 0;
        auto start = chrono::high_resolution_clock::now();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::high_resolution_clock::now();
        // Code to be timed
        newquickselect(arr, N / 2);
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "Time taken by quickSelect: " << duration.count() << " microseconds" << endl;
        cout << "Median: " << arr[N / 2] << endl;
        N += 10000;
    }
    // 从N=10000到N=100000，步长为10000，顺序数据
    cout << "Ordered data" << endl;
    N = 10000;
    while (N <= 100000)
    {
        vector<int> arr;
        for (int i = 0; i < N; i++)
        {
            arr.push_back(i);
        }
        int ans = 0;
        auto start = chrono::high_resolution_clock::now();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::high_resolution_clock::now();
        // Code to be timed
        newquickselect(arr, N / 2);
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "Time taken by quickSelect: " << duration.count() << " microseconds" << endl;
        cout << "Median: " << arr[N / 2] << endl;
        N += 10000;
    }
    // 从N=10000到N=100000，步长为10000，逆序数据
    cout << "Reverse data" << endl;
    N = 10000;
    while (N <= 100000)
    {
        vector<int> arr;
        for (int i = N; i > 0; i--)
        {
            arr.push_back(i);
        }
        int ans = 0;
        auto start = chrono::high_resolution_clock::now();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::high_resolution_clock::now();
        // Code to be timed
        newquickselect(arr, N / 2);
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "Time taken by quickSelect: " << duration.count() << " microseconds" << endl;
        cout << "Median: " << arr[N / 2] << endl;
        N += 10000;
    }
}
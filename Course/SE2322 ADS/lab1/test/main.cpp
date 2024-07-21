#include "radix_tree.hpp"
#include "compressed_radix_tree.hpp"
#include "RBTree.h"
#include <set>
#include "util.hpp"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <chrono>
#define RUN_TIME 60

using namespace std;

void runRadixTreeTest(int workloadType);
void runCompressedRadixTreeTest(int workloadType);
void runRedBlackTreeTest(int workloadType);
void runNewRedBlackTreeTest(int workloadType);
void calculateDelay(vector<int> dataVector);

int main()
{
    // workloadType: 1, 2, 3
    runRadixTreeTest(1);
    runRadixTreeTest(2);
    runRadixTreeTest(3);

    //workloadType: 1, 2, 3
    runCompressedRadixTreeTest(1);
    runCompressedRadixTreeTest(2);
    runCompressedRadixTreeTest(3);

    // This RBtree use set to implement
    // //workloadType: 1, 2, 3
    // runRedBlackTreeTest(1);
    // runRedBlackTreeTest(2);
    // runRedBlackTreeTest(3);

    //workloadType: 1, 2, 3
    runNewRedBlackTreeTest(1);
    runNewRedBlackTreeTest(2);
    runNewRedBlackTreeTest(3);

    return 0;
}

// 定义测试函数
void runRadixTreeTest(int workloadType)
{
    cout << endl;
    cout << "Running RadixTree test..." << endl;

    // 创建RadixTree对象
    Tree *RadixTree;
    RadixTree = new class RadixTree();

    // 初始化RadixTree，将1000个随机数插入到RadixTree中
    //cout << "Inserting 1000 random numbers into RadixTree..." << endl;
    for (int i = 0; i < 1000; i++)
    {
        RadixTree->insert(zipf());
    }

    // 初始化记录表，用于记录插入、查找、删除操作的时间
    vector<int> insertTime;
    vector<int> findTime;
    vector<int> deleteTime;
    insertTime.clear();
    findTime.clear();
    deleteTime.clear();

    // 输出当前是处于那种工作负载
    cout << "Current workload type: " << workloadType << endl;

    // 定义开始时间和结束时间，运行时间为60s
    auto start = std::chrono::steady_clock::now();
    auto target = start + std::chrono::seconds(RUN_TIME);

    // 根据不同的工作负载类型，进行不同的操作
    switch (workloadType)
    {
    case 1:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-1: 50% find, 50% insert
            int random = zipf() / 2;
            if (random % 2 == 0)
            {
                //记录find操作的时间
                int index = zipf();
                auto findStart = std::chrono::steady_clock::now();
                RadixTree->find(index);
                auto findEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入findTime中
                findTime.push_back(static_cast<int>((findEnd - findStart).count()));
            }
            else
            {
                //记录insert操作的时间
                int index = zipf();
                auto insertStart = std::chrono::steady_clock::now();
                RadixTree->insert(index);
                auto insertEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入insertTime中
                insertTime.push_back(static_cast<int>((insertEnd - insertStart).count()));
            }
        }
        break;
    case 2:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-2: 100% find
            //记录find操作的时间
            int index = zipf();
            auto findStart = std::chrono::steady_clock::now();
            RadixTree->find(index);
            auto findEnd = std::chrono::steady_clock::now();
            // 转化为int类型，并存入findTime中
            findTime.push_back(static_cast<int>((findEnd - findStart).count()));
        }
        break;
    case 3:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-3: 50% find, 25% insert, 25% delete
            // 生成0到1的随机数
            double random = static_cast<double>(rand()) / RAND_MAX;
            if (random < 0.5)
            {
                //50%执行find操作
                //记录find操作的时间
                int index = zipf();
                auto findStart = std::chrono::steady_clock::now();
                RadixTree->find(index);
                auto findEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入findTime中
                findTime.push_back(static_cast<int>((findEnd - findStart).count()));
            }
            else if (random >= 0.5 && random < 0.75)
            {
                //25%执行insert操作
                //记录insert操作的时间
                int index = zipf();
                auto insertStart = std::chrono::steady_clock::now();
                RadixTree->insert(index);
                auto insertEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入insertTime中
                insertTime.push_back(static_cast<int>((insertEnd - insertStart).count()));
            }
            else
            {
                //25%执行delete操作
                //记录delete操作的时间
                int index = zipf();
                auto deleteStart = std::chrono::steady_clock::now();
                RadixTree->remove(index);
                auto deleteEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入deleteTime中
                deleteTime.push_back(static_cast<int>((deleteEnd - deleteStart).count()));
            }
        }
        break;
    default:
        break;
    }
    //cout << "RadixTree test finished." << endl << endl;

    cout << endl;
    cout << "Calculating insert delay..." << endl;
    calculateDelay(insertTime);

    cout << endl;
    cout << "Calculating find delay..." << endl;
    calculateDelay(findTime);

    cout << endl;
    cout << "Calculating delete delay..." << endl;
    calculateDelay(deleteTime);
}

void runCompressedRadixTreeTest(int workloadType)
{
    cout << endl;
    cout << "Running CompressedRadixTree test..." << endl;

    // 创建CompressedRadixTree对象
    Tree *CompressedRadixTree;
    CompressedRadixTree = new class CompressedRadixTree();

    // 初始化CompressedRadixTree，将1000个随机数插入到CompressedRadixTree中
    // cout << "Inserting 1000 random numbers into CompressedRadixTree..." << endl;
    for (int i = 0; i < 1000; i++)
    {
        CompressedRadixTree->insert(zipf());
    }

    // 初始化记录表，用于记录插入、查找、删除操作的时间
    vector<int> insertTime;
    vector<int> findTime;
    vector<int> deleteTime;
    insertTime.clear();
    findTime.clear();
    deleteTime.clear();

    // 输出当前是处于那种工作负载
    cout << "Current workload type: " << workloadType << endl;

    // 定义开始时间和结束时间，运行时间为60s
    auto start = std::chrono::steady_clock::now();
    auto target = start + std::chrono::seconds(RUN_TIME);

    // 根据不同的工作负载类型，进行不同的操作
    switch (workloadType)
    {
    case 1:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-1: 50% find, 50% insert
            int random = zipf() / 2;
            if (random % 2 == 0)
            {
                // 记录find操作的时间
                int index = zipf();
                auto findStart = std::chrono::steady_clock::now();
                CompressedRadixTree->find(index);
                auto findEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入findTime中
                findTime.push_back(static_cast<int>((findEnd - findStart).count()));
            }
            else
            {
                // 记录insert操作的时间
                int index = zipf();
                auto insertStart = std::chrono::steady_clock::now();
                CompressedRadixTree->insert(index);
                auto insertEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入insertTime中
                insertTime.push_back(static_cast<int>((insertEnd - insertStart).count()));
            }
        }
        break;
    case 2:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-2: 100% find
            // 记录find操作的时间
            int index = zipf();
            auto findStart = std::chrono::steady_clock::now();
            CompressedRadixTree->find(index);
            auto findEnd = std::chrono::steady_clock::now();
            // 转化为int类型，并存入findTime中
            findTime.push_back(static_cast<int>((findEnd - findStart).count()));
        }
        break;
    case 3:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-3: 50% find, 25% insert, 25% delete
            // 生成0到1的随机数
            double random = static_cast<double>(rand()) / RAND_MAX;
            if (random < 0.5)
            {
                // 50%执行find操作
                // 记录find操作的时间
                int index = zipf();
                auto findStart = std::chrono::steady_clock::now();
                CompressedRadixTree->find(index);
                auto findEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入findTime中
                findTime.push_back(static_cast<int>((findEnd - findStart).count()));
            }
            else if (random >= 0.5 && random < 0.75)
            {
                // 25%执行insert操作
                // 记录insert操作的时间
                int index = zipf();
                auto insertStart = std::chrono::steady_clock::now();
                CompressedRadixTree->insert(index);
                auto insertEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入insertTime中
                insertTime.push_back(static_cast<int>((insertEnd - insertStart).count()));
            }
            else
            {
                // 25%执行delete操作
                // 记录delete操作的时间
                int index = zipf();
                auto deleteStart = std::chrono::steady_clock::now();
                CompressedRadixTree->remove(index);
                auto deleteEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入deleteTime中
                deleteTime.push_back(static_cast<int>((deleteEnd - deleteStart).count()));
            }
        }
        break;
    default:
        break;
    }
    // cout << "CompressedRadixTree test finished." << endl << endl;

    cout << endl;
    cout << "Calculating insert delay..." << endl;
    calculateDelay(insertTime);

    cout << endl;
    cout << "Calculating find delay..." << endl;
    calculateDelay(findTime);

    cout << endl;
    cout << "Calculating delete delay..." << endl;
    calculateDelay(deleteTime);
}

void runRedBlackTreeTest(int workloadType)
{
    cout << endl;
    cout << "Running RedBlackTree test..." << endl;

    // 创建RedBlackTree对象
    set<int> RedBlackTree;

    // 初始化RedBlackTree，将1000个随机数插入到RedBlackTree中
    // cout << "Inserting 1000 random numbers into RedBlackTree..." << endl;
    for (int i = 0; i < 1000; i++)
    {
        RedBlackTree.insert(zipf());
    }

    // 初始化记录表，用于记录插入、查找、删除操作的时间
    vector<int> insertTime;
    vector<int> findTime;
    vector<int> deleteTime;
    insertTime.clear();
    findTime.clear();
    deleteTime.clear();

    // 输出当前是处于那种工作负载
    cout << "Current workload type: " << workloadType << endl;

    // 定义开始时间和结束时间，运行时间为60s
    auto start = std::chrono::steady_clock::now();
    auto target = start + std::chrono::seconds(RUN_TIME);

    // 根据不同的工作负载类型，进行不同的操作
    switch (workloadType)
    {
    case 1:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-1: 50% find, 50% insert
            int random = zipf() / 2;
            if (random % 2 == 0)
            {
                // 记录find操作的时间
                int index = zipf();
                auto findStart = std::chrono::steady_clock::now();
                RedBlackTree.find(index);
                auto findEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入findTime中
                findTime.push_back(static_cast<int>((findEnd - findStart).count()));
            }
            else
            {
                // 记录insert操作的时间
                int index = zipf();
                auto insertStart = std::chrono::steady_clock::now();
                RedBlackTree.insert(index);
                auto insertEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入insertTime中
                insertTime.push_back(static_cast<int>((insertEnd - insertStart).count()));
            }
        }
        break;
    case 2:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-2: 100% find
            // 记录find操作的时间
            int index = zipf();
            auto findStart = std::chrono::steady_clock::now();
            RedBlackTree.find(index);
            auto findEnd = std::chrono::steady_clock::now();
            // 转化为int类型，并存入findTime中
            findTime.push_back(static_cast<int>((findEnd - findStart).count()));
        }
        break;
    case 3:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-3: 50% find, 25% insert, 25% delete
            // 生成0到1的随机数
            double random = static_cast<double>(rand()) / RAND_MAX;
            if (random < 0.5)
            {
                // 50%执行find操作
                // 记录find操作的时间
                int index = zipf();
                auto findStart = std::chrono::steady_clock::now();
                RedBlackTree.find(index);
                auto findEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入findTime中
                findTime.push_back(static_cast<int>((findEnd - findStart).count()));
            }
            else if (random >= 0.5 && random < 0.75)
            {
                // 25%执行insert操作
                // 记录insert操作的时间
                int index = zipf();
                auto insertStart = std::chrono::steady_clock::now();
                RedBlackTree.insert(index);
                auto insertEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入insertTime中
                insertTime.push_back(static_cast<int>((insertEnd - insertStart).count()));
            }
            else
            {
                // 25%执行delete操作
                // 记录delete操作的时间
                int index = zipf();
                auto deleteStart = std::chrono::steady_clock::now();
                RedBlackTree.erase(index);
                auto deleteEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入deleteTime中
                deleteTime.push_back(static_cast<int>((deleteEnd - deleteStart).count()));
            }
        }
        break;
    default:
        break;
    }
    // cout << "RedBlackTree test finished." << endl << endl;

    cout << endl;
    cout << "Calculating insert delay..." << endl;
    calculateDelay(insertTime);

    cout << endl;
    cout << "Calculating find delay..." << endl;
    calculateDelay(findTime);

    cout << endl;
    cout << "Calculating delete delay..." << endl;
    calculateDelay(deleteTime);
}

void calculateDelay(vector<int> dataVector){
    if(dataVector.size() == 0){
        cout << "No data in the vector." << endl;
        return;
    }
    // Calculate average delay
    double averageDelay = 0.0;
    for (auto time : dataVector)
    {
        averageDelay += time;
    }
    averageDelay /= dataVector.size();

    // Calculate P50 delay
    sort(dataVector.begin(), dataVector.end());
    int p50Delay = dataVector[dataVector.size() / 2];

    // Calculate P90 delay
    int p90Delay = dataVector[static_cast<int>(dataVector.size() * 0.9)];

    // Calculate P99 delay
    int p99Delay = dataVector[static_cast<int>(dataVector.size() * 0.99)];

    // Print the calculated parameters
    cout << fixed << setprecision(3) << "Average Delay: " << averageDelay << endl;
    cout << "P50 Delay: " << p50Delay << endl;
    cout << "P90 Delay: " << p90Delay << endl;
    cout << "P99 Delay: " << p99Delay << endl;
}

void runNewRedBlackTreeTest(int workloadType)
{
    cout << endl;
    cout << "Running NewRedBlackTree test..." << endl;

    // 创建NewRedBlackTree对象
    RedBlackTree<int> NewRedBlackTree;

        // 初始化NewRedBlackTree，将1000个随机数插入到NewRedBlackTree中
        // cout << "Inserting 1000 random numbers into NewRedBlackTree..." << endl;
    for (int i = 0; i < 1000; i++)
    {
        NewRedBlackTree.insert(zipf());
    }

    // 初始化记录表，用于记录插入、查找、删除操作的时间
    vector<int> insertTime;
    vector<int> findTime;
    vector<int> deleteTime;
    insertTime.clear();
    findTime.clear();
    deleteTime.clear();

    // 输出当前是处于那种工作负载
    cout << "Current workload type: " << workloadType << endl;

    // 定义开始时间和结束时间，运行时间为60s
    auto start = std::chrono::steady_clock::now();
    auto target = start + std::chrono::seconds(RUN_TIME);

    // 根据不同的工作负载类型，进行不同的操作
    switch (workloadType)
    {
    case 1:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-1: 50% find, 50% insert
            int random = zipf() / 2;
            if (random % 2 == 0)
            {
                // 记录find操作的时间
                int index = zipf();
                auto findStart = std::chrono::steady_clock::now();
                NewRedBlackTree.find(index);
                auto findEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入findTime中
                findTime.push_back(static_cast<int>((findEnd - findStart).count()));
            }
            else
            {
                // 记录insert操作的时间
                int index = zipf();
                auto insertStart = std::chrono::steady_clock::now();
                NewRedBlackTree.insert(index);
                auto insertEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入insertTime中
                insertTime.push_back(static_cast<int>((insertEnd - insertStart).count()));
            }
        }
        break;
    case 2:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-2: 100% find
            // 记录find操作的时间
            int index = zipf();
            auto findStart = std::chrono::steady_clock::now();
            NewRedBlackTree.find(index);
            auto findEnd = std::chrono::steady_clock::now();
            // 转化为int类型，并存入findTime中
            findTime.push_back(static_cast<int>((findEnd - findStart).count()));
        }
        break;
    case 3:
        while (std::chrono::steady_clock::now() < target)
        {
            // workload-3: 50% find, 25% insert, 25% delete
            // 生成0到1的随机数
            double random = static_cast<double>(rand()) / RAND_MAX;
            if (random < 0.5)
            {
                // 50%执行find操作
                // 记录find操作的时间
                int index = zipf();
                auto findStart = std::chrono::steady_clock::now();
                NewRedBlackTree.find(index);
                auto findEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入findTime中
                findTime.push_back(static_cast<int>((findEnd - findStart).count()));
            }
            else if (random >= 0.5 && random < 0.75)
            {
                // 25%执行insert操作
                // 记录insert操作的时间
                int index = zipf();
                auto insertStart = std::chrono::steady_clock::now();
                NewRedBlackTree.insert(index);
                auto insertEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入insertTime中
                insertTime.push_back(static_cast<int>((insertEnd - insertStart).count()));
            }
            else
            {
                // 25%执行delete操作
                // 记录delete操作的时间
                int index = zipf();
                auto deleteStart = std::chrono::steady_clock::now();
                NewRedBlackTree.remove(index);
                auto deleteEnd = std::chrono::steady_clock::now();
                // 转化为int类型，并存入deleteTime中
                deleteTime.push_back(static_cast<int>((deleteEnd - deleteStart).count()));
            }
        }
        break;
    default:
        break;
    }
    // cout << "NewRedBlackTree test finished." << endl << endl;

    cout << endl;
    cout << "Calculating insert delay..." << endl;
    calculateDelay(insertTime);

    cout << endl;
    cout << "Calculating find delay..." << endl;
    calculateDelay(findTime);

    cout << endl;
    cout << "Calculating delete delay..." << endl;
    calculateDelay(deleteTime);
}
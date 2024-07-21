#include <iostream>
#include <cstdint>
#include <string>
#include <chrono>
#include "test.h"

/*
常规分析：测试 PUT、GET、和 DEL 三种操作的吞吐量（每秒执行
操作个数）和平均时延（每一个操作完成需要的时间）
*/
void test_1(KVStore &store);

/*
 索引缓存与 Bloom Filter 的效果：对比以下三种设计中，GET 操作的
平均时延
a) 内存中不缓存 SSTable 的任何信息，从磁盘中访问 SSTable 的索引，
在找到 offset 之后读取数据。
b) 内存中只缓存了 SSTable 的索引信息（ <Key，offset，vlen> 元组），
通过二分查找从 SSTable 的索引中找到 offset，并在磁盘中读取对
应的值。
c) 内存中缓存 SSTable 的 Bloom Filter 和索引，先通过 Bloom Filter 判
断一个键值是否可能在一个 SSTable 中，如果存在再利用二分查
找，否则直接查看下一个 SSTable 的索引。
*/
void test_2(KVStore &store);

/*
 Compaction 的影响：不断插入数据的情况下，统计插入操作的时延
的变化情况。测试需要表现出 Compaction 对时延的影响，即当某次
插入操作触发 Compaction 之后该插入的latency应该会明显上升。
*/
void test_3(KVStore &store);

/*
Bloom Filter 大小的影响：Bloom Filter 过大会使得一个 SSTable 中索
引数据较少，进而导致 SSTable 合并操作频繁；Bloom Filter 过小又会
导致其 false positive 的几率过大，辅助查找的效果不好。你可以尝试
不同大小的 Bloom Filter 并保持 SSTable 大小不变，比较系统 Get、
Put 操作性能的差异并思考原因。
*/
void test_4(KVStore &store);

int main()
{
    KVStore store("./data", "./data/vlog");
    test_1(store);
    test_2(store);
    test_3(store);
    test_4(store);
    return 0;
}

/*
常规分析：测试 PUT、GET、和 DEL 三种操作的吞吐量（每秒执行
操作个数）和平均时延（每一个操作完成需要的时间）
包括Get、Put、Delete、Scan操作的延迟，你需要测出不同数据大小时的操作延迟，为了测试的合理性，你应当对每个数据大小测量然后计算出平均延迟
包括Get、Put、Delete、Scan操作的吞吐，意思是指系统单位时间内能够相应的请求的次数，显然，在展示你测试的时候你需要指明Key、Value的大小（注意是数据的大小，并不是具体的值）
先进行Put操作，然后进行Get操作，然后进行Scan操作，最后进行Delete操作
*/
void test_1(KVStore &store)
{
    // 输出提示信息、测试开始
    std::cout << std::endl;
    std::cout << "Test 1 started!" << std::endl;
    std::cout << std::endl;

    // 初始化store
    store.reset();

    // 设置基础操作次数
    uint64_t operation_times = 1024 * 8;

    // 一共进行5轮测试、每轮测试的数据大小为operation_times 乘 2 的 ( i - 1 ) 次方，分别是 1024 * 8、1024 * 16、1024 * 32、1024 * 64、1024 * 128 次
    for (int i = 1; i <= 5; i++)
    {
        // 初始化store
        store.reset();

        // 设置本轮的数据大小
        uint64_t operation_times_this_turn = operation_times * (1 << (i - 1));

        // 初始化时间累加器
        std::chrono::duration<double> diff_put(0);
        std::chrono::duration<double> diff_get(0);
        std::chrono::duration<double> diff_scan(0);
        std::chrono::duration<double> diff_del(0);

        // 测试Put操作
        for (uint64_t j = 1; j <= operation_times_this_turn; j++)
        {
            // 这里为了除去外层循环对计时的影响，将计时放在内层循环中，并将计时结果累加
            auto start_put = std::chrono::high_resolution_clock::now();
            // 这里为了体现数据规模对测试性能的影响，将Value的大小设置为1000
            // 即每个Value都是1000个字符s组成的字符串
            store.put(j, std::string(1000, 's'));
            auto end_put = std::chrono::high_resolution_clock::now();
            diff_put += end_put - start_put;
        }

        // 测试Get操作
        for (uint64_t j = 1; j <= operation_times_this_turn; j++)
        {
            auto start_get = std::chrono::high_resolution_clock::now();
            store.get(j);
            auto end_get = std::chrono::high_resolution_clock::now();
            diff_get += end_get - start_get;
        }

        // 测试Scan操作，一共进行许多次Scan操作，每次的范围为[j, j + 7]
        std::list<std::pair<uint64_t, std::string>> list;
        for (uint64_t j = 1; j <= operation_times_this_turn; j += 8)
        {
            auto start_scan = std::chrono::high_resolution_clock::now();
            store.scan(j, j + 7, list);
            auto end_scan = std::chrono::high_resolution_clock::now();
            diff_scan += end_scan - start_scan;
        }

        // 测试Delete操作
        for (uint64_t j = 1; j <= operation_times_this_turn; j++)
        {
            auto start_del = std::chrono::high_resolution_clock::now();
            store.del(j);
            auto end_del = std::chrono::high_resolution_clock::now();
            diff_del += end_del - start_del;
        }

        // 计算延迟和吞吐量
        double latency_put = diff_put.count() / operation_times_this_turn;
        double latency_get = diff_get.count() / operation_times_this_turn;
        double latency_scan = diff_scan.count() / (operation_times_this_turn / 8);
        double latency_del = diff_del.count() / operation_times_this_turn;

        double throughput_put = operation_times_this_turn / diff_put.count();
        double throughput_get = operation_times_this_turn / diff_get.count();
        double throughput_scan = (operation_times_this_turn / 8) / diff_scan.count();
        double throughput_del = operation_times_this_turn / diff_del.count();

        // 输出测试结果
        std::cout << "Test " << i << " finished!" << std::endl;
        std::cout << "Data size: " << operation_times_this_turn << std::endl;
        std::cout << "Put: latency = " << std::chrono::duration_cast<std::chrono::microseconds>(diff_put).count() / operation_times_this_turn << " us, throughput = " << throughput_put << " ops/sec" << std::endl;
        std::cout << "Get: latency = " << std::chrono::duration_cast<std::chrono::microseconds>(diff_get).count() / operation_times_this_turn << " us, throughput = " << throughput_get << " ops/sec" << std::endl;
        std::cout << "Scan: latency = " << std::chrono::duration_cast<std::chrono::microseconds>(diff_scan).count() / (operation_times_this_turn / 8) << " us, throughput = " << throughput_scan << " ops/sec" << std::endl;
        std::cout << "Delete: latency = " << std::chrono::duration_cast<std::chrono::microseconds>(diff_del).count() / operation_times_this_turn << " us, throughput = " << throughput_del << " ops/sec" << std::endl;
    }

    // 输出提示信息，测试结束
    std::cout << std::endl;
    std::cout << "Test 1 finished!" << std::endl;
    std::cout << std::endl;
}

/*
 索引缓存与 Bloom Filter 的效果：对比以下三种设计中，GET 操作的
平均时延
a) 内存中不缓存 SSTable 的任何信息，从磁盘中访问 SSTable 的索引，
在找到 offset 之后读取数据。
b) 内存中只缓存了 SSTable 的索引信息（ <Key，offset，vlen> 元组），
通过二分查找从 SSTable 的索引中找到 offset，并在磁盘中读取对
应的值。
c) 内存中缓存 SSTable 的 Bloom Filter 和索引，先通过 Bloom Filter 判
断一个键值是否可能在一个 SSTable 中，如果存在再利用二分查
找，否则直接查看下一个 SSTable 的索引。
*/
void test_2(KVStore &store)
{
    // 输出提示信息、测试开始
    std::cout << std::endl;
    std::cout << "Test 2 started!" << std::endl;
    std::cout << std::endl;

    // 初始化store
    store.reset();

    // 设置总操作数为1024 * 2次
    uint64_t operation_times = 1024 * 2;

    // 不断插入数据，每次插入的数据为长度为1000的字符串s
    for (uint64_t i = 1; i <= operation_times; i++)
    {
        store.put(i, std::string(1000, 's'));
    }

    // b) 内存中只缓存了 SSTable 的索引信息（ <Key，offset，vlen> 元组），通过二分查找从 SSTable 的索引中找到 offset，并在磁盘中读取对应的值
    std::chrono::duration<double> total_time_b = std::chrono::duration<double>(0);
    for (uint64_t i = 1; i <= operation_times; i++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        store.get_without_bloom_filter(i);
        auto end = std::chrono::high_resolution_clock::now();
        total_time_b += end - start;
    }
    std::cout << "Get without bloom filter: latency = " << std::chrono::duration_cast<std::chrono::nanoseconds>(total_time_b).count() / operation_times << " ns, throughput = " << operation_times / total_time_b.count() << " ops/sec" << std::endl;

    // c) 内存中缓存 SSTable 的 Bloom Filter 和索引，先通过 Bloom Filter 判断一个键值是否可能在一个 SSTable 中，
    // 如果存在再利用二分查找，否则直接查看下一个 SSTable 的索引
    std::chrono::duration<double> total_time_c = std::chrono::duration<double>(0);
    for (uint64_t i = 1; i <= operation_times; i++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        store.get(i);
        auto end = std::chrono::high_resolution_clock::now();
        total_time_c += end - start;
    }
    std::cout << "Get with bloom filter: latency = " << std::chrono::duration_cast<std::chrono::nanoseconds>(total_time_c).count() / operation_times << " ns, throughput = " << operation_times / total_time_c.count() << " ops/sec" << std::endl;

    // a) 内存中不缓存 SSTable 的任何信息，从磁盘中访问 SSTable 的索引，在找到 offset 之后读取数据
    std::chrono::duration<double> total_time_a = std::chrono::duration<double>(0);
    for (uint64_t i = 1; i <= operation_times; i++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        store.get_from_disk(i);
        auto end = std::chrono::high_resolution_clock::now();
        total_time_a += end - start;
    }
    std::cout << "Get from disk: latency = " << std::chrono::duration_cast<std::chrono::nanoseconds>(total_time_a).count() / operation_times << " ns, throughput = " << operation_times / total_time_a.count() << " ops/sec" << std::endl;

    std::cout << std::endl;
    std::cout << "Test 2 finished!" << std::endl;
    std::cout << std::endl;
}

/*
 Compaction 的影响：不断插入数据的情况下，统计插入操作的时延
的变化情况。测试需要表现出 Compaction 对时延的影响，即当某次
插入操作触发 Compaction 之后该插入的latency应该会明显上升。
不断插入数据的情况下，统计每秒钟处理的PUT请求个数（即吞吐量），并绘制其随时间变化的折线图，测试需要表现出compaction对吞吐量的影响。
*/
void test_3(KVStore &store)
{
    // 输出提示信息、测试开始
    std::cout << std::endl;
    std::cout << "Test 3 started!" << std::endl;
    std::cout << std::endl;

    // 初始化store
    store.reset();

    // 定义总操作数为1024 * 2次
    uint64_t operation_times = 1024 * 2;

    // 不断插入数据，每次插入的数据为长度为1024的字符串s，记录每次put所需要的时间、存放在vector中
    std::vector<std::chrono::duration<double>> diff_put;
    std::chrono::duration<double> total_put_time(0);
    for (uint64_t i = 1; i <= operation_times; i++)
    {
        auto start_put = std::chrono::high_resolution_clock::now();
        store.put(i, std::string(1024, 's'));
        auto end_put = std::chrono::high_resolution_clock::now();
        auto diff = end_put - start_put;
        diff_put.push_back(diff);
        total_put_time += diff;
    }

    // 打印每次put所需要的时间
    for (uint64_t i = 0; i < operation_times; i++)
    {
        std::cout << "Put " << i + 1 << ": " << std::chrono::duration_cast<std::chrono::nanoseconds>(diff_put[i]).count() << " ns" << std::endl;
    }

    // 输出PUT操作的吞吐量
    std::cout << "Put: throughput = " << operation_times / total_put_time.count() << " ops/sec" << std::endl;

    // 输出提示信息，测试结束
    std::cout << std::endl;
    std::cout << "Test 3 finished!" << std::endl;
    std::cout << std::endl;
}

/*
Bloom Filter 大小的影响：Bloom Filter 过大会使得一个 SSTable 中索
引数据较少，进而导致 SSTable 合并操作频繁；Bloom Filter 过小又会
导致其 false positive 的几率过大，辅助查找的效果不好。你可以尝试
不同大小的 Bloom Filter 并保持 SSTable 大小不变，比较系统 Get、
Put 操作性能的差异并思考原因。
*/
void test_4(KVStore &store)
{
    // 输出提示信息、测试开始
    std::cout << std::endl;
    std::cout << "Test 4 started!" << std::endl;
    std::cout << std::endl;

    // 输出当前Bloom Filter的大小
    std::cout << "Current Bloom Filter size: " << sstable_bloom_filter_size << std::endl;

    // 初始化store
    store.reset();

    // 设置总操作数为1024 * 8次
    uint64_t operation_times = 1024 * 8;

    // 进行Put操作，每次Put操作的Value大小为1000，即每个Value都是1000个字符s组成的字符串
    std::chrono::duration<double> total_put_time(0);
    for (uint64_t i = 1; i <= operation_times; i++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        store.put(i, std::string(1000, 's'));
        auto end = std::chrono::high_resolution_clock::now();
        total_put_time += end - start;
    }
    std::cout << "Put: latency = " << std::chrono::duration_cast<std::chrono::nanoseconds>(total_put_time).count() / operation_times << " ns, throughput = " << operation_times / total_put_time.count() << " ops/sec" << std::endl;

    // 进行Get操作
    std::chrono::duration<double> total_get_time(0);
    for (uint64_t i = 1; i <= operation_times; i++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        store.get(i);
        auto end = std::chrono::high_resolution_clock::now();
        total_get_time += end - start;
    }
    std::cout << "Get: latency = " << std::chrono::duration_cast<std::chrono::nanoseconds>(total_get_time).count() / operation_times << " ns, throughput = " << operation_times / total_get_time.count() << " ops/sec" << std::endl;

    // 输出提示信息，测试结束
    std::cout << std::endl;
    std::cout << "Test 4 finished!" << std::endl;
    std::cout << std::endl;
}
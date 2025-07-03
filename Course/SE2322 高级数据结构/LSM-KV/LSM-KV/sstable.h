#pragma once

#include "config.h"
#include "bloomFilter.h"
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

/*
本项目中一个 SSTable 的大小不超过
16 kB，但应尽量接近。如图2所示，每个 SSTable 文件的结构分为三个部分。
1) Header 用于存放元数据，按顺序分别为该 SSTable 的时间戳（无符号
64 位整型），SSTable 中键值对的数量（无符号 64 位整型），键最小
值和最大值（无符号 64 位整型），共占用 32 B。
2) Bloom Filter 用来快速判断 SSTable 中是否存在某一个键值，本项目要
求 Bloom Filter 的大小为 8 kB （8192 字节），hash 函数使用给定的
Murmur3，将 hash 得到的 128-bit 结果分为四个无符号 32 位整型使用。
超出Bloom Filter 长度的结果要进行取余。
3) <Key, Offset, Vlen>元组，用来存储有序的索引数据，包含所有的
Key、Key 对应的 Value 在 vLog 文件中的偏移量 offset (无符号 64 位
整数)以及值的长度（无符号 32 位整数）。
*/

// SSTable 文件的头部，包含时间戳、键值对数量、键的最小值和最大值
struct SSTableHeader
{
    uint64_t timestamp; // 时间戳
    uint64_t num;       // 键值对的数量
    uint64_t min_key;   // 键的最小值
    uint64_t max_key;   // 键的最大值
};

// SSTable 文件中的元组，包含键、值在 vLog 文件中的偏移量和值的长度
struct SSTableTuple
{
    uint64_t key;    // 键
    uint64_t offset; // 值在 vLog 文件中的偏移量
    uint32_t vlen;   // 值的长度
};

// SSTable 类，用于操作 SSTable 文件
class SSTable
{
public:
    std::string SSTable_path;         // SSTable 文件路径
    SSTableHeader header;             // SSTable 的头部
    std::vector<SSTableTuple> tuples; // SSTable 的元组
    bloomFilter *bloom_filter;        // Bloom Filter

    SSTable(std::string SSTable_path);                                                                     // 从 SSTable 文件路径构造 SSTable 对象
    SSTable() {}                                                                                           // 默认构造函数
    ~SSTable();                                                                                            // 析构函数
    SSTableTuple getTupleByBinarySearch(uint64_t key);                                                     // 通过二分查找在一个SSTable中获取元组、因为SSTable是有序的
    void scan(key_type K1, key_type K2, std::map<uint64_t, std::string> &scan_map, std::string vLog_path); // 扫描SSTable，将key值在[K1, K2]之间的键值对存入scan_map中
};
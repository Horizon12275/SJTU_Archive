#include "sstable.h"

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

SSTable::SSTable(std::string SSTable_path) : SSTable_path(SSTable_path)
{
    // 读取SSTable文件，恢复header和tuples的值
    std::ifstream in(SSTable_path, std::ios::binary);
    // 如果文件不存在，则初始化并创建
    if (!in)
    {
        this->header.timestamp = 0;
        this->header.num = 0;
        this->header.min_key = 0;
        this->header.max_key = 0;
        this->tuples.clear();
        this->bloom_filter = nullptr;
        std::ofstream out(SSTable_path, std::ios::binary);
        out.close();
        return;
    }
    // 如果文件存在，则依次读取header，bloom_filter和tuples的值
    // 读取header的值
    in.read((char *)&this->header.timestamp, sizeof(uint64_t));
    in.read((char *)&this->header.num, sizeof(uint64_t));
    in.read((char *)&this->header.min_key, sizeof(uint64_t));
    in.read((char *)&this->header.max_key, sizeof(uint64_t));

    // 读取bloom_filter的值
    this->bloom_filter = new bloomFilter();
    std::vector<unsigned char> binary_filter(sstable_bloom_filter_size);
    in.read((char *)&binary_filter[0], sstable_bloom_filter_size);
    this->bloom_filter->binary_to_filter(binary_filter);

    // 读取tuples的值
    int n = this->header.num;
    for (int i = 0; i < n; i++)
    {
        SSTableTuple tuple;
        in.read((char *)&tuple.key, sizeof(uint64_t));
        in.read((char *)&tuple.offset, sizeof(uint64_t));
        in.read((char *)&tuple.vlen, sizeof(uint32_t));
        this->tuples.push_back(tuple);
    }
    in.close();
}

// 析构函数、释放指针
SSTable::~SSTable()
{
    // 释放bloom_filter指针
    if (this->bloom_filter != nullptr)
    {
        delete this->bloom_filter;
    }
}

// 通过二分查找在一个SSTable中获取元组、因为SSTable是有序的
SSTableTuple SSTable::getTupleByBinarySearch(uint64_t key)
{
    // 二分查找
    int left = 0, right = this->tuples.size() - 1;
    while (left <= right)
    {
        int mid = (left + right) / 2;
        if (this->tuples[mid].key == key)
        {
            // 如果找到、返回元组
            return this->tuples[mid];
        }
        else if (this->tuples[mid].key < key)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    // 如果没找到、返回一个空的元组，其中key为UINT64_MAX，offset为UINT64_MAX，vlen为UINT32_MAX，即无效元组
    SSTableTuple tuple;
    tuple.key = UINT64_MAX;
    tuple.offset = UINT64_MAX;
    tuple.vlen = UINT32_MAX;
    return tuple;
}

// 扫描SSTable，将key值在[K1, K2]之间所有的键值对存入scan_map中
void SSTable::scan(key_type K1, key_type K2, std::map<uint64_t, std::string> &scan_map, std::string vLog_path)
{
    // 这里可以首先用SSTable存储的最大值和最小值进行判断、如果K1大于最大值或者K2小于最小值则直接返回
    if (K1 > this->header.max_key || K2 < this->header.min_key)
    {
        return;
    }
    // 接着用二分查找找到第一个大于等于K1的元组
    int left = 0, right = this->tuples.size() - 1;
    int start = -1;
    while (left <= right)
    {
        int mid = (left + right) / 2;
        if (this->tuples[mid].key >= K1)
        {
            start = mid;
            right = mid - 1;
        }
        else
        {
            left = mid + 1;
        }
    }
    // 从start开始扫描、直到找到第一个大于K2的元组
    for (size_t i = start; i < this->tuples.size(); i++)
    {
        if (this->tuples[i].key > K2)
        {
            break;
        }
        // 如果元组的key在[K1, K2]之间、则将其存入scan_map中
        SSTableTuple tuple = this->tuples[i];
        // 如果元组的vlen是0，那么这个元组是一个删除标记，要在scan_map中删除这个key
        if (tuple.vlen == 0)
        {
            scan_map.erase(tuple.key);
            continue;
        }
        std::string value;
        // 从vLog文件中读取value
        std::ifstream in(vLog_path, std::ios::binary);
        in.seekg(tuple.offset);
        char *buf = new char[tuple.vlen];
        in.read(buf, tuple.vlen);
        value = std::string(buf, tuple.vlen);
        delete[] buf;
        in.close();
        scan_map[tuple.key] = value;
    }
}
#pragma once
#include <stdint.h>
#include <vector>
#include "MurmurHash3.h"
#include "config.h"

class bloomFilter
{
private:
    std::vector<bool> filter;                                // 布隆过滤器
    static const uint64_t k = 4;                             // 哈希函数的个数，这里把128位的哈希结果分为4个、那就是4个哈希函数
    static const uint64_t m = sstable_bloom_filter_size * 8; // 布隆过滤器的大小，8192字节（8KB），转换为位数，即1024*8*8位

public:
    // 构造函数，初始化布隆过滤器
    bloomFilter()
    {
        filter.resize(m);
        std::fill(filter.begin(), filter.end(), false);
    }

    ~bloomFilter() {}

    // 插入key，将key哈希k次，将哈希值对m取模，将对应位置置为true
    void insert(uint64_t key)
    {
        uint32_t hash[4] = {0};
        MurmurHash3_x64_128(&key, sizeof(key), 1, hash);
        for (uint64_t i = 0; i < k; i++)
        {
            filter[hash[i] % m] = true;
        }
    }

    // 查找key，将key哈希k次，将哈希值对m取模，如果对应位置为false则返回false
    bool find(uint64_t key)
    {
        uint32_t hash[4] = {0};
        MurmurHash3_x64_128(&key, sizeof(key), 1, hash);
        for (uint64_t i = 0; i < k; i++)
        {
            if (!filter[hash[i] % m])
            {
                return false;
            }
        }
        return true;
    }

    // 得到对应的布隆过滤器的转化后的二进制序列
    // 从filter的开头开始遍历、将每8个bool值转化为一个unsigned char值
    // 第0个bool值是最高位，第7个bool值是最低位
    // binary_filter的大小共m/8个unsigned char
    std::vector<unsigned char> get_binary_filter()
    {
        std::vector<unsigned char> binary_filter(m / 8, 0);
        for (uint64_t i = 0; i < m; i += 8)
        {
            unsigned char byte = 0;
            for (uint64_t j = 0; j < 8; j++)
            {
                byte |= filter[i + j] << (7 - j);
            }
            binary_filter[i / 8] = byte;
        }
        return binary_filter;
    }

    // 将二进制序列转化为布隆过滤器中的值，注意binary_filter是unsigned char，而filter是bool的vector，所以filter的大小是binary_filter的8倍
    // binary_filter的大小共m/8个unsigned char
    // 从binary_filter的开头开始遍历、将每一个unsigned char值转化为8个bool值
    // 将unsigned char的值转化为一个8位的bool值的vector，然后一一赋值给filter
    void binary_to_filter(std::vector<unsigned char> binary_filter)
    {
        for (uint64_t i = 0; i < m; i += 8)
        {
            unsigned char byte = binary_filter[i / 8];
            for (uint64_t j = 0; j < 8; j++)
            {
                filter[i + j] = (byte >> (7 - j)) & 1;
            }
        }
    }
};
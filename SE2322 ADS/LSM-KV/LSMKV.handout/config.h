#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <deque>
#include <map>
#include <algorithm>
#include <fstream>
#include <chrono>

// 键的类型，本项目中键的类型为无符号 64 位整型
using key_type = uint64_t;

// 值的类型，本项目中值的类型为字符串
using value_type = std::string;

// 删除标记
#define delete_tag "~DELETED~"

// MemTable 的最大level（文件夹数）
#define sstable_max_level 16

// SSTable 头部的大小
#define sstable_header_size 32 // 32B

// Bloom Filter 的大小，默认为8KB，其他大小用于测试
// #define sstable_bloom_filter_size 1024 * 12 // 12KB
#define sstable_bloom_filter_size 1024 * 8 // 8KB
// #define sstable_bloom_filter_size 1024 * 4 // 4KB
// #define sstable_bloom_filter_size 1024 * 2 // 2KB
// #define sstable_bloom_filter_size 1024 * 1 // 1KB
// #define sstable_bloom_filter_size 0 // 0KB

// SSTable 单个元组的大小
#define sstable_tuple_size 20 // 20B

// SSTable 的最大大小
#define sstable_max_size 1024 * 16 // 16KB

// vLog entry 的magic number
#define vlog_entry_magic 0xff
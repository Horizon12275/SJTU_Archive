#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <string>
#include <random>
#include <limits>
#include <cstdlib>
#include <map>
#include "config.h"
#include "sstable.h"
#include "bloomFilter.h"

// 跳表节点的定义
struct MemTableNode
{
	key_type key;			// 键值
	value_type value;		// 值
	MemTableNode **forward; // 指向下一个节点的指针数组

	// 构造函数，初始化键值对
	MemTableNode(int level, key_type k, const value_type &v) : key(k), value(v)
	{
		forward = new MemTableNode *[level + 1](); // 初始化为nullptr
	}

	// 析构函数
	~MemTableNode()
	{
		delete[] forward; // 释放forward数组
	}
};

// 跳表的定义
class MemTable
{
private:
	MemTableNode *header; // 存储的是头指针
	double p;			  // 跳表的概率
	int maxLevel;		  // 跳表的最大层数
	int currentLevel;	  // 当前跳表的层数
	int randomLevel();	  // 随机生成一个层数
	size_t size;		  // MemTable转化为SSTable后的大小

public:
	// 构造函数
	explicit MemTable(double p = 0.25);

	// 析构函数
	~MemTable();

	// 将key和value插入MemTable中、如果key已经存在则更新value
	void put(key_type key, const value_type &val);

	// 得到key对应的value，如果key不存在则返回空
	value_type get(key_type key) const;

	// 重置MemTable
	void reset();

	// 扫描MemTable，将key值在[K1, K2]之间的键值对存入scan_map中
	void scan(key_type K1, key_type K2, std::map<uint64_t, std::string> &scan_map);

	// 得到MemTable的大小
	size_t getSize() const;

	// 遍历MemTable，将存储的所有键值对存入vector中
	std::vector<std::pair<key_type, value_type>> getAll() const;

	// bool类型，用于判断MemTable是否为空
	bool isEmpty() const;
};

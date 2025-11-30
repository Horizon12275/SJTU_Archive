#pragma once

#include "kvstore_api.h"
#include "memtable.h"
#include "config.h"
#include "utils.h"
#include "sstable.h"
#include "vlog.h"
#include <fstream>
#include <map>
#include <iostream>
#include "bloomFilter.h"

class KVStore : public KVStoreAPI
{
private:
	MemTable *memtable;								   // MemTable，是一个跳表
	std::deque<SSTable *> sstables[sstable_max_level]; // SSTable 文件缓存，越在deque前面的SSTable文件越新，从第0层开始，共16个level，每层存储的最大SSTable数是2^(level+1)个
	vLog *vlog;										   // vLog 文件，用于维护 head 和 tail，并不实际存储数据
	std::string data_dir;							   // 对应的数据存储目录
	std::string vlog_path;							   // 对应的vlog存储目录
	uint64_t time_stamp;							   // 当前记录的时间戳

public:
	// 构造函数、传入的参数是数据存储目录和vlog存储目录，进行初始化与硬盘数据的读取
	KVStore(const std::string &dir, const std::string &vlog);

	// 析构函数、用于释放已经分配的指针和内存、防止内存泄漏
	~KVStore();

	// 重载接口函数，用于实现对应的功能
	// put方法传入键值对，将其插入到MemTable中
	void put(uint64_t key, const std::string &s) override;

	// get方法传入键，返回对应的值
	std::string get(uint64_t key) override;

	// del方法传入键，删除对应的键值对
	bool del(uint64_t key) override;

	// reset方法，用于重置KVStore，将其状态重置为初始状态，并清空硬盘上的数据
	void reset() override;

	// scan方法，传入两个键，返回这两个键之间的所有键值对
	void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) override;

	// gc方法，传入一个chunk_size，用于进行垃圾回收
	void gc(uint64_t chunk_size) override;

private:
	// 以下是上述接口中所会用到的工具函数
	// memtable_to_SSTable_and_vLog方法，用于将MemTable中的数据写入到SSTable和vLog的硬盘中
	void memtable_to_SSTable_and_vLog();

	// 对当前level层中all_sst_files中的字符串进行排序预处理，按照时间戳的大小进行排序、即时间戳大的代表新的
	// 时间戳小的代表旧的，然后按照all_sst_files的从头至尾的顺序、依次把小时间戳的排在前面、大时间戳的排在后面
	void sort_all_sst_files_on_one_level(std::vector<std::string> &all_sst_files);

	// 合并操作：若 Level 0 层中的文件数量超过限制，则开始进行合并操作。
	void try_compaction();

public:
	// 最后是为了测试写的两个不同的GET函数
	// get_from_disk不利用缓存、直接读硬盘中的SSTable进行查找
	std::string get_from_disk(uint64_t key);

	// get_without_bloom_filter不利用布隆过滤器、直接读取SSTable进行查找
	std::string get_without_bloom_filter(uint64_t key);
};

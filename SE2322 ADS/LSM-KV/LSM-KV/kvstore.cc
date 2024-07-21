#include "kvstore.h"
#include <string>

// 初始化 KVStore
KVStore::KVStore(const std::string &dir, const std::string &vlog) : KVStoreAPI(dir, vlog)
{
	// std::cout << "magic is:" << (char)((uint8_t)vlog_entry_magic) << std::endl;

	// 设定文件存储目录，vlog文件路径
	this->data_dir = dir;
	this->vlog_path = vlog;

	// 如果data_dir目录不存在，创建data_dir目录
	if (!utils::dirExists(data_dir))
		utils::_mkdir(data_dir);

	// 创建最大时间戳局部变量、减少内存访问
	uint64_t max_time_stamp = 1;

	// 创建Level_0到sstable_max_level的目录
	for (int i = 0; i < sstable_max_level; i++)
	{
		std::string level_dir = data_dir + "/Level_" + std::to_string(i);
		if (!utils::dirExists(level_dir))
			utils::_mkdir(level_dir);
		else
		{
			// 如果有目录、那么将目录下的所有SSTable文件缓存到内存中，这里要求将时间戳最新的SSTable文件放在deque的最前面
			std::vector<std::string> all_sst_files;
			// 这里检查一下，先打印出当前层数
			// std::cout << "Level_" << i << std::endl;
			// 获取目录下的所有文件名，存入all_sst_files
			utils::scanDir(level_dir, all_sst_files);
			// 对当前level层中all_sst_files中的字符串进行排序预处理，按照时间戳的大小进行排序、即时间戳大的代表新的
			// 时间戳小的代表旧的，然后按照all_sst_files的从头至尾的顺序、依次把小时间戳的排在前面、大时间戳的排在后面
			sort_all_sst_files_on_one_level(all_sst_files);
			// 排序完了之后便对该level层的所有SSTable文件进行遍历和缓存
			for (auto it = all_sst_files.begin(); it != all_sst_files.end(); it++)
			{
				std::string sst_path = level_dir + "/" + *it;

				// 这里检查一下、打印出当前层该SSTable文件的路径
				// std::cout << sst_path << std::endl;

				// 每次把读到的sst差到sstables当前层的deque的最前面，因为deque的最前面是时间戳最新的
				// 而all_sst_files中的文件是按照时间戳从小到大排列的、所以每次读到的文件都是尚未读完中的时间戳最小的
				SSTable *sst = new SSTable(sst_path);
				this->sstables[i].push_front(sst);
				// 将当前的时间戳更新为最大的时间戳
				max_time_stamp = std::max(max_time_stamp, sst->header.timestamp);
			}
			// std::cout << std::endl;
		}
	}

	// 将当前的时间戳更新为读到的SSTable中最大的时间戳
	this->time_stamp = max_time_stamp + 1;

	// 初始化 MemTable
	this->memtable = new MemTable();

	// 读取vLog文件，恢复tail和head的值，如果vLog文件不存在则创建一个新的vLog文件
	this->vlog = new vLog(vlog_path);
}

// 析构函数、需要正确地释放所有内存、否则会造成内存泄漏
KVStore::~KVStore()
{
	// 如果在正常关闭时，应将 MemTable 中的所有数据写入 SSTable 和 vLog
	if (!memtable->isEmpty())
	{
		memtable_to_SSTable_and_vLog();
	}

	// 释放memtable的指针
	delete memtable;

	// 释放每一层中sstables的内存
	for (int i = 0; i < sstable_max_level; i++)
	{
		for (auto it = this->sstables[i].begin(); it != this->sstables[i].end(); it++)
		{
			delete *it;
		}
	}

	// 释放vlog的指针
	delete vlog;
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 * 对于 PUT 操作，首先尝试在 MemTable 中进行插入。由于
 * MemTable 使用跳表维护，因此如果其中存在 Key 的记录，则在
 * MemTable 中进行覆盖（即替换）而非插入。同时，如果在插入或覆
 * 盖之后，MemTable 的大小超出限制（注意这里指的是 MemTable 转
 * 换为 SSTable之后大小超过 16kB，请注意不要计入值 value 的大小），
 * 则暂不进行插入或覆盖，而是首先将 MemTable 中的数据转换成
 * vLog entry 写入 vLog 中，并依次得到每个 entry 在 vLog 文件中的
 * offset。接着，依据得到的 offset 生成相应的 SSTable 保存在 Level 0 层
 * 中。若 Level 0 层的文件数量超出限制，则开始进行合并操作。合并
 * 操作可按照前文所述具体方法。在这些操作完毕之后，再进行 Key
 * 的插入。
 */
void KVStore::put(uint64_t key, const std::string &s)
{
	// 如果其中存在 Key 的记录，则在 MemTable 中进行覆盖（即替换）而非插入
	if (memtable->getSize() + sizeof(SSTableTuple::key) + sizeof(SSTableTuple::offset) + sizeof(SSTableTuple::vlen) <= sstable_max_size)
	{
		memtable->put(key, s);
		return;
	}
	// 如果在插入或覆盖之后，MemTable 的大小超出限制，则暂不进行插入或覆盖，而是首先将 MemTable 中的数据转换成 vLog entry 写入 vLog 中
	// 并依次得到每个 entry 在 vLog 文件中的 offset，接着，依据得到的 offset 生成相应的 SSTable 保存在 Level 0 层中
	// std::cout << "memtable size: " << memtable->getSize() << std::endl;
	memtable_to_SSTable_and_vLog();

	// 若 Level 0 层的文件数量超出限制，则开始进行合并操作，这里先进行检查与尝试合并操作
	try_compaction();

	// 重置MemTable
	memtable->reset();

	// 在这些操作完毕之后，再进行 Key 的插入
	// std::cout << "put key: " << key << " value: " << s << std::endl;
	memtable->put(key, s);
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 * 对于 GET 操作，首先从 MemTable 中进行查找，当查找到键 K
 * 所对应的记录之后结束。
 * 若 MemTable 中不存在键 Key，则先从内存里逐层查看缓存的每
 * 一个 SSTable，先用 Bloom Filter 中判断 Key 是否在当前 SSTable 中，
 * 如果可能存在则用二分查找在索引中找到对应的<key, offset, vlen>元
 * 组。之后即可从vLog中根据 offset 及 vlen 取出 value。如果找遍了所
 * 有的层都没有这个 Key 值，则说明该 Key 不存在。
 */
std::string KVStore::get(uint64_t key)
{
	// 首先从 MemTable 中进行查找，当查找到键 K 所对应的记录之后结束
	// std::cout << "get key: " << key << std::endl;
	std::string result = memtable->get(key);
	// std::cout << "result: " << result << std::endl;

	// 如果result不为空，说明在MemTable中找到了
	if (result == delete_tag) // 如果找到的是删除标记，则说明该Key已被删除，返回空字符串
		return "";
	if (result != "") // 如果找到的是正常的值，则直接返回
		return result;

	// 如果在MemTable中没有找到，则从SSTable中查找，这里要从level_0到level_15依次查找。
	// 因为先查到的SSTable文件是时间戳最新的，相当于覆盖了时间戳旧的数据内容
	for (int i = 0; i < sstable_max_level; i++)
	{
		// 按照时间戳的降序对sstables进行排序
		std::sort(this->sstables[i].begin(), this->sstables[i].end(), [](SSTable *a, SSTable *b)
				  { return a->header.timestamp > b->header.timestamp; });
		// 从SSTable的缓存中查找key1到key2之间的键值对
		for (auto it = this->sstables[i].begin(); it != this->sstables[i].end(); it++)
		{
			// 先用 Bloom Filter 中判断 Key 是否在当前 SSTable 中
			if ((*it)->bloom_filter->find(key))
			{
				// 如果可能存在则用二分查找在索引中找到对应的<key, offset, vlen>元组
				SSTableTuple tuple = (*it)->getTupleByBinarySearch(key);
				// 如果返回的元组、其三个值都是最大值，那么就是表示没找到的空元组、此时直接在下一个SSTable中寻找
				if (tuple.key == UINT64_MAX && tuple.offset == UINT64_MAX && tuple.vlen == UINT32_MAX)
					continue;
				// 如果找到了元组的key值
				if (tuple.key == key)
				{
					// 如果元组的vlen是0，说明该key已被删除，返回空字符串
					if (tuple.vlen == 0)
					{
						// std::cout << "check vlen == 0" << std::endl;
						return "";
					}

					// 之后即可从vLog中根据 offset 及 vlen 取出 value
					std::string value = vlog->getValue(tuple.offset, tuple.vlen);
					return value;
				}
			}
		}
	}
	// 如果找遍了所有的层都没有这个 Key 值，则说明该 Key 不存在，返回空字符串
	// std::cout << "check null" << std::endl;
	return result;
}

/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 * 由于我们不能修改 SSTable 中的内容，我们需要一种特殊的方
 * 式处理键值对的删除操作。首先，我们查找键 Key 。如果未查找到
 * 记录，则不需要进行删除操作，返回 false；若搜索到记录，则在
 * MemTable 中再插入一条记录，表示键 Key 被删除了，并返回 true。
 * 我们称此特殊的记录为“删除标记”，其键为 Key ，值为特殊字符
 * 串“~DELETED~”（测试中不会出现以此为值的正常记录）。当读
 * 操作读到了一个“删除标记”时，说明该 Key 已被删除。
 * 在使用 MemTable 生成 SSTable 以及 vLog 时，只需在 SSTable中
 * 将该 Key 对应的 vlen 设为 0 而不需要写 vLog ，在 GET 操作时，即
 * 可通过 vlen 是否为 0 来判断是否为删除操作。你也可以通过其他方
 * 式来处理删除，但需要保证操作的正确性。在执行合并操作时，根
 * 据时间戳将相同键的多个记录进行合并，通常不需要对 vlen 为 0 的
 * 记录作特殊处理。唯一一个例外，是在最后一层中合并时，所有
 * vlen 为 0 的记录应被丢弃。
 */
// 有以下几种返回false的条件
// 第一种是先在MemTable和后在所有的SSTable中找到了key，但是这个key对应的value是delete_tag
// 第二种是先在MemTable和后在所有的SSTable都没有找到key，返回的字符串为空字符串时
// 除了这两种情况、先在MemTable和后在所有的SSTable中找到了key、且key对应的value不是delete_tag时、都返回true，
// 同时在MemTable中插入一个key为key、value为delete_tag的键值对
// 这里为了正确性、简化为了如下的del版本
bool KVStore::del(uint64_t key)
{
	// 如果key不存在
	if (get(key) == "")
		return false;
	// 否则插入删除标记
	put(key, delete_tag);
	return true;
}

// 以下是先前逻辑的错误返回逻辑、以供参考
// bool KVStore::del(uint64_t key)
// {
// 	// 先在MemTable中查找
// 	std::string result = memtable->get(key);
// 	// 如果在MemTable中找到了对应的value是delete_tag，则说明该key已被删除，返回false
// 	if (result == delete_tag)
// 		return false;
// 	// 如果在MemTable中找到了对应的value，但value不是delete_tag，则说明该key存在，利用put操作覆盖，返回true
// 	if (result != "")
// 	{
// 		memtable->put(key, delete_tag);
// 		return true;
// 	}

// 	// 如果在MemTable中没有找到，则从SSTable中查找，这里要从level_0到level_15依次查找。
// 	// 因为先查到的SSTable文件是时间戳最新的，相当于覆盖了时间戳旧的数据内容
// 	for (int i = 0; i < sstable_max_level; i++)
// 	{
// 		for (auto it = this->sstables[i].begin(); it != this->sstables[i].end(); it++)
// 		{
// 			// 先用 Bloom Filter 中判断 Key 是否在当前 SSTable 中
// 			if ((*it)->bloom_filter->find(key))
// 			{
// 				// 如果可能存在则用二分查找在索引中找到对应的<key, offset, vlen>元组
// 				SSTableTuple tuple = (*it)->getTupleByBinarySearch(key);
// 				// 如果返回的元组、其三个值都是最大值，那么就是表示没找到的空元组、此时直接在下一个SSTable中寻找
// 				if (tuple.key == UINT64_MAX && tuple.offset == UINT64_MAX && tuple.vlen == UINT32_MAX)
// 					continue;
// 				// 如果找到了元组的key值
// 				if (tuple.key == key)
// 				{
// 					// 如果元组的vlen是0，说明该key已被删除，返回false
// 					if (tuple.vlen == 0)
// 						return false;
// 					// 如果vlen不为0，说明该key存在，利用put操作覆盖在MemTable中插入一个key为key、value为delete_tag的键值对，返回true
// 					put(key, delete_tag);
// 					return true;
// 				}
// 			}
// 		}
// 	}
// 	return false;
// }

// 以下为仅有MemTable时的正确返回逻辑、以供参考
// bool KVStore::del(uint64_t key)
// {
// 	// if (memtable->get(key) == "" || memtable->get(key) == delete_tag)
// 	// 	return false;
// 	// memtable->put(key, delete_tag);
// 	// return true;
// }

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 * 本项目中所实现的键值存储系统在启动时，需检查现有的数据
 * 目录中各层 SSTable 文件，并在内存中构建相应的缓存；同时还需要
 * 恢复 tail 和 head 的正确值，具体来说：head 的值就是当前文件的大
 * 小；tail 则需要首先定位到文件空洞后的第一个 Magic，接着进行 crc
 * 校验，如果校验通过则该 Magic 的位置即为 tail 的值，否则寻找下一
 * 个 Magic 并进行校验，直到校验通过，则对应的 Magic 位置即为 tail
 * 的正确值。因此，其启动后应读取到上次系统运行所记录的 SSTable
 * 数据及 vLog 文件。在调用 reset() 函数时，其应将所有层的 SSTable
 * 文件（以及表示层的目录）、vLog 文件删除，清除内存中 MemTable
 * 和缓存等数据，将 tail 和 head 的值置 0，使键值存储系统恢复到空的
 * 状态。同时，系统在正常关闭时（可以实现在析构函数里面），应将
 * MemTable 中的所有数据写入 SSTable 和 vLog（类似于 MemTable 满
 * 了时的操作）。
 */
void KVStore::reset()
{
	// 清除所有层的SSTable文件以及表示层的目录
	for (int i = 0; i < sstable_max_level; i++)
	{
		std::string level_dir = data_dir + "/Level_" + std::to_string(i);
		if (utils::dirExists(level_dir))
		{
			std::vector<std::string> all_sst_files;
			utils::scanDir(level_dir, all_sst_files);
			for (auto it = all_sst_files.begin(); it != all_sst_files.end(); it++)
			{
				std::string sst_path = level_dir + "/" + *it;
				utils::rmfile(sst_path.c_str());
			}
			utils::rmdir(level_dir);
		}
	}

	// 清除缓存中的sstables，要对每一层都清除
	for (int i = 0; i < sstable_max_level; i++)
	{
		for (auto it = this->sstables[i].begin(); it != this->sstables[i].end(); it++)
		{
			delete *it;
		}
	}

	// 重置sstables，要对每一层都重置
	for (int i = 0; i < sstable_max_level; i++)
	{
		this->sstables[i].clear();
	}

	// 清除内存中 MemTable
	this->memtable->reset();

	// 删除vlog文件
	utils::rmfile(vlog_path.c_str());

	// 删除data_dir的目录
	utils::rmdir(data_dir);

	// 释放vlog内存
	delete vlog;

	// 置空vlog指针
	vlog = nullptr;

	// 初始化时间戳为1
	this->time_stamp = 1;
}

/**
 * Return a list including all the key-value pair between key1 and key2.
 * keys in the list should be in an ascending order.
 * An empty string indicates not found.
 * SCAN 操作需要返回一个 std::list<K, V>，其中按递增顺序存放了
 * 所有键在 K1 和 K2 （左右均包含）的键值对。SCAN 有多种实现方法，
 * 可以扫描 MemTable 和所有 SSTable 中的数据，也可以通过多个指针进
 * 行堆排序。此处具体实现不做要求。但是不能通过 for (i = K1; i <= K2;
 * ++i) 的方式实现，因为测试中 K2- K1 可能非常大。
 */
// 具体流程：
// 1.扫描sstables（先扫描 SSTable，注意要从后往前扫描，因为 SSTable 文件缓存越在 deque 前面的 SSTable 文件越新）
// 2.扫描memtable（有序的）
// 3.把扫描结果存入list进行返回
void KVStore::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list)
{
	// 用于存放扫描结果
	std::map<uint64_t, std::string> scan_map;

	// 先扫描 SSTable，注意要在缓存的sstables中从后往前扫描，因为 SSTable 文件缓存越在 deque 前面的 SSTable 文件越新
	// 我们需要先扫描时间戳旧的、这样我们在扫描到时间戳新的时候就可以直接覆盖掉
	// 同时、对于层级、也要从大的层开始扫描、因为大的层的时间戳更小、就更旧、操作会被后续的覆盖掉，保证了正确性
	for (int i = sstable_max_level - 1; i >= 0; i--)
	{
		// 对该层的sstables进行排序,按照时间戳的降序
		std::sort(this->sstables[i].begin(), this->sstables[i].end(), [](SSTable *a, SSTable *b)
				  { return a->header.timestamp > b->header.timestamp; });
		for (auto it = this->sstables[i].rbegin(); it != this->sstables[i].rend(); it++)
		{
			// 从SSTable中扫描key1到key2之间的键值对
			(*it)->scan(key1, key2, scan_map, vlog_path);
		}
	}

	// 再扫描 MemTable，将 MemTable 中的数据加入 scan_map ，注意这里会自动覆盖掉先前扫描SSTable得到的结果、符合逻辑
	// 因为 MemTable 中的数据是最新的
	memtable->scan(key1, key2, scan_map);

	// 将scan_map中的数据加入list，list中的数据是有序的
	for (auto it = scan_map.begin(); it != scan_map.end(); it++)
	{
		list.push_back(std::make_pair(it->first, it->second));
	}
}

/**
 * This reclaims space from vLog by moving valid value and discarding invalid value.
 * chunk_size is the size in byte you should AT LEAST recycle.
 */
/*
垃圾回收（Garbage Collection）
在没有使用键值分离优化时，垃圾回收在合并 SSTable 时即可自动完成，
然而当引入了键值分离之后，value 被单独存在 vLog 中，在合并操作时不
会删除，因此需要额外的垃圾回收机制回收过期的（被删除或修改）的
vLog entry。
在本项目中，你需要实现在代码框架中预定义接口的 GC 函数，测试函
数会在特定的时间调用该函数。GC 函数的主要包含以下参数：
1) chunk_size：本轮 GC 扫描的 vLog 大小（严格不小于，例如
chunk_size = 1024 Byte，而 vLog 头部的 10 个 entry 加起来大小为 1023
Byte，则需要再多扫描一个 entry ）。
GC的流程包括以下几步：
1) 逐个扫描 vLog 头部 GCSize 大小内的 vLog entry，使用该 vLog entry
的Key 在 LSM Tree 中查找最新的记录，比较其 offset 是否指向该
vLog entry。
2) 如果是，表明该 vLog entry 仍然记录的是最新数据，则将该 vLog
entry 重新插入到 MemTable 中。
3) 如果不是，表明该 vLog entry 记录的是过期的数据，不做处理。
4) 扫描完成后，不论此时 MemTable 是否可以容纳更多的数据（即继
续插入新数据后仍然可以使得转化成的 SSTable 的大小满足要求），
只要 MemTable 中含有数据，就需要主动将 MemTable 写入 SStable
和vLog（类似于 MemTable 满了时的操作）。扫描到的未过期的数据
会在这一步重新 append 到 vLog 头部。
5) 使用 de_alloc_file() 帮助函数对扫描过的 vLog 文件区域打洞（详见第
7 节介绍）
*/
void KVStore::gc(uint64_t chunk_size)
{
	// 以二进制读方式打开vLog文件
	std::ifstream vLog_file(vlog_path, std::ios::binary | std::ios::in);
	// 检查文件是否打开成功
	if (!vLog_file.is_open())
	{
		std::cerr << "Error: failed to open vLog file" << std::endl;
		return;
	}

	// 设置当前的head和tail
	uint64_t head = vlog->head; // head同时也是当前文件的大小
	uint64_t tail = vlog->tail;
	uint64_t n = std::min(tail + chunk_size, head); // 本轮GC扫描的vLog大小，保证不超过文件大小

	// 从tail开始读取vLog文件
	vLog_file.seekg(tail);

	// std::cout << tail << " " << n << std::endl;

	// 逐个扫描 vLog 头部 GCSize 大小内的 vLog entry
	while (vLog_file.tellg() < n)
	{
		// 读取vLog entry
		vlogEntry entry;
		// 读取vLog entry的magic
		vLog_file.read((char *)&entry.magic, sizeof(entry.magic));
		// std::cout << "magic in vlog gc is:" << (char)((uint8_t)entry.magic) << std::endl;
		// 读取vLog entry的checksum
		vLog_file.read((char *)&entry.checksum, sizeof(entry.checksum));
		// 读取vLog entry的key
		vLog_file.read((char *)&entry.key, sizeof(entry.key));
		// 读取vLog entry的vlen
		vLog_file.read((char *)&entry.vlen, sizeof(entry.vlen));
		// 在这时设置当前vlog文件中该entry的offset
		uint64_t this_entry_offset_in_vlog = vLog_file.tellg();
		// 读取vLog entry的value
		char value[entry.vlen];
		vLog_file.read(value, entry.vlen);
		entry.value = std::string(value, entry.vlen);

		// 使用该 vLog entry 的 Key 在 LSM Tree 中查找最新的记录
		// 首先在MemTable中查找
		std::string result = memtable->get(entry.key);
		// 如果在MemTable中找到了对应的value是delete_tag，则说明该key已被删除，不做处理，直接读下一个entry
		if (result == delete_tag)
			continue;
		// 如果在MemTable中找到了对应的value，但value不是delete_tag，也不是空字符串，则说明该key存在，不做处理，直接读下一个entry
		if (result != "")
			continue;

		bool found = false;
		uint64_t final_offset = UINT64_MAX;
		// 然后在SSTable中查找，从level_0到level_15依次查找，比较其 offset 是否与该 vLog entry 的 offset 相等
		for (int i = 0; i < sstable_max_level; i++)
		{
			for (auto it = this->sstables[i].begin(); it != this->sstables[i].end(); it++)
			{
				// 先用 Bloom Filter 中判断 Key 是否在当前 SSTable 中
				if ((*it)->bloom_filter->find(entry.key))
				{
					// 如果可能存在则用二分查找在索引中找到对应的<key, offset, vlen>元组
					SSTableTuple tuple = (*it)->getTupleByBinarySearch(entry.key);
					// 如果返回的元组、其三个值都是最大值，那么就是表示没找到的空元组、此时直接在下一个SSTable中寻找
					if (tuple.key == UINT64_MAX && tuple.offset == UINT64_MAX && tuple.vlen == UINT32_MAX)
						continue;
					// 如果找到了元组的key值
					if (tuple.key == entry.key)
					{
						// 如果元组的vlen是0，说明该key已被删除，不做处理，直接读下一个entry
						if (tuple.vlen == 0)
						{
							found = true;
							final_offset = 0;
						}
						else
						{
							found = true;
							final_offset = tuple.offset;
						}
					}
				}
				if (found)
					break;
			}
			if (found)
				break;
		}
		// 如果元组的offset与该 vLog entry 的 offset 相等，表明该 vLog entry 仍然记录的是最新数据，则将该 vLog entry 重新插入到 MemTable 中
		if (final_offset == this_entry_offset_in_vlog)
		{
			put(entry.key, entry.value);
		}
	}

	// 更新tail 也就是gc后的tail 如果读到了文件末尾 则tail为filesize
	vlog->tail = vLog_file.tellg() == -1 ? head : (size_t)vLog_file.tellg();

	// 强制把memtable转换为sstable和vlog并写入缓存和磁盘
	if (!memtable->isEmpty())
	{
		memtable_to_SSTable_and_vLog();
	}

	// 对扫描过的区域打洞
	if (tail < vlog->tail)
		utils::de_alloc_file(vlog_path, tail, vlog->tail - tail);

	// std::cout << "tail after gc: " << vlog->tail << std::endl;

	// 关闭文件
	vLog_file.close();
}

//--------------------------------------------------------------------------------
// 以下是辅助函数的具体编写：
//--------------------------------------------------------------------------------

/**
 * 将 MemTable 中的数据转换成 SSTable 保存在 Level 0 层，并将相应的 vLog entry 写入 vLog 中
 */
void KVStore::memtable_to_SSTable_and_vLog()
{
	// 如果在插入或覆盖之后，MemTable 的大小超出限制，则暂不进行插入或覆盖，而是首先将 MemTable 中的数据转换成 vLog entry 写入 vLog 中
	// 并依次得到每个 entry 在 vLog 文件中的 offset，接着，依据得到的 offset 生成相应的 SSTable 保存在 Level 0 层中

	// 首先将 MemTable 中的数据转换成 vLog entry 写入 vLog 中
	// 依次得到每个 entry 在 vLog 文件中的 offset

	// 如果data_dir目录不存在，创建data_dir目录
	if (!utils::dirExists(data_dir))
		utils::_mkdir(data_dir);

	// 如果对应的路径下没有SSTable的目录、则创建Level_0到sstable_max_level的目录
	for (int i = 0; i < sstable_max_level; i++)
	{
		std::string level_dir = data_dir + "/Level_" + std::to_string(i);
		if (!utils::dirExists(level_dir))
			utils::_mkdir(level_dir);
	}

	// 如果对应的vlog路径下没有vlog文件、那么先创建一个vlog文件
	if (vlog == nullptr)
	{
		vlog = new vLog(vlog_path);
	}

	// 以二进制形式打开 vLog 文件，并在文件末尾追加写入
	std::ofstream vLog_file(vlog_path, std::ios::binary | std::ios::out | std::ios::app);

	// 新建 SSTable 文件
	std::string SSTable_path = data_dir + "/Level_0/SSTable_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "_" + std::to_string(time_stamp) + ".sst";
	SSTable *new_sstable = new SSTable(SSTable_path);

	// 获取 MemTable 中的所有键值对
	std::vector<std::pair<key_type, value_type>> allInMemTable = memtable->getAll();

	// 准备SSTable的Header
	SSTableHeader new_header;
	new_header.timestamp = time_stamp;
	time_stamp++;
	new_header.num = allInMemTable.size();
	new_header.min_key = allInMemTable[0].first;
	new_header.max_key = allInMemTable[allInMemTable.size() - 1].first;

	// 将Header写入new_sstable
	new_sstable->header = new_header;

	// 准备SSTable的Bloom Filter
	new_sstable->bloom_filter = new bloomFilter();

	// 依次将键值对写入 vLog 文件与 SSTable 文件
	// 这里先写入 vLog 文件，再写入 SSTable 文件
	for (auto it = allInMemTable.begin(); it != allInMemTable.end(); it++)
	{
		/* 当读
		 * 操作读到了一个“删除标记”时，说明该 Key 已被删除。
		 * 在使用 MemTable 生成 SSTable 以及 vLog 时，只需在 SSTable中
		 * 将该 Key 对应的 vlen 设为 0 而不需要写 vLog ，
		 */
		if (it->second == delete_tag)
		{
			// 生成 SSTable 的元组
			SSTableTuple tuple;
			tuple.key = it->first; // 键
			tuple.vlen = 0;		   // 值的长度
			tuple.offset = 0;	   // 值在 vLog 文件中的偏移量
			// 插入到bloom filter中
			new_sstable->bloom_filter->insert(tuple.key);
			new_header.min_key = std::min(new_header.min_key, tuple.key);
			new_header.max_key = std::max(new_header.max_key, tuple.key);

			// 将元组加入sstable的元组数组
			new_sstable->tuples.push_back(tuple);
			continue;
		}

		// 最终要写入 vLog 文件的 vlogEntry的二进制序列，依次存储 Magic, Checksum, Key, vlen, Value
		std::vector<unsigned char> vlogEntryBinary;

		// 生成 vLog entry
		vlogEntry *entry = new vlogEntry();
		entry->magic = vlog_entry_magic;
		// std::cout << "entry->magic: " << (char)entry->magic << std::endl;
		entry->key = it->first;
		entry->vlen = it->second.size();
		entry->value = it->second;

		// 存储由 key, vlen, value 拼接而成的二进制序列
		std::vector<unsigned char> key_vlen_value;
		key_vlen_value.insert(key_vlen_value.end(), (unsigned char *)&entry->key, (unsigned char *)&entry->key + sizeof(entry->key));
		key_vlen_value.insert(key_vlen_value.end(), (unsigned char *)&entry->vlen, (unsigned char *)&entry->vlen + sizeof(entry->vlen));
		key_vlen_value.insert(key_vlen_value.end(), entry->value.begin(), entry->value.end());

		// 生成 Checksum
		entry->checksum = utils::crc16(key_vlen_value);

		// 将 entry 的 Magic, Checksum, Key, vlen, Value 依次存入 vlogEntryBinary
		vlogEntryBinary.push_back(entry->magic);
		vlogEntryBinary.insert(vlogEntryBinary.end(), (unsigned char *)&entry->checksum, (unsigned char *)&entry->checksum + sizeof(entry->checksum));
		vlogEntryBinary.insert(vlogEntryBinary.end(), (unsigned char *)&entry->key, (unsigned char *)&entry->key + sizeof(entry->key));
		vlogEntryBinary.insert(vlogEntryBinary.end(), (unsigned char *)&entry->vlen, (unsigned char *)&entry->vlen + sizeof(entry->vlen));
		vlogEntryBinary.insert(vlogEntryBinary.end(), entry->value.begin(), entry->value.end());

		// 将 vlogEntryBinary 写入 vLog 文件
		vLog_file.write((char *)&vlogEntryBinary[0], vlogEntryBinary.size());

		// 在Bloom Filter中插入对应的键
		new_sstable->bloom_filter->insert(entry->key);

		// 生成 SSTable 的元组
		SSTableTuple tuple;
		tuple.key = entry->key;						   // 键
		tuple.vlen = entry->vlen;					   // 值的长度
		tuple.offset = vLog_file.tellp() - tuple.vlen; // 值在 vLog 文件中的偏移量

		// 将元组加入sstable的元组数组
		new_sstable->tuples.push_back(tuple);

		// 释放 entry
		delete entry;
	}

	// 维护vlog的head为当前文件指针位置
	vlog->head = vLog_file.tellp();

	// 关闭 vLog 文件
	vLog_file.close();

	// 接着将SSTable写到硬盘里去，以二进制输出的方式、先写header、再写entry，注意顺序
	// 以二进制形式打开 SSTable 文件，并在文件末尾追加写入
	std::ofstream SSTable_file(SSTable_path, std::ios::binary | std::ios::out | std::ios::app);

	// 创建SSTable的Header的二进制序列
	std::vector<unsigned char> headerBinary;
	headerBinary.insert(headerBinary.end(), (unsigned char *)&new_sstable->header.timestamp, (unsigned char *)&new_sstable->header.timestamp + sizeof(new_sstable->header.timestamp));
	headerBinary.insert(headerBinary.end(), (unsigned char *)&new_sstable->header.num, (unsigned char *)&new_sstable->header.num + sizeof(new_sstable->header.num));
	headerBinary.insert(headerBinary.end(), (unsigned char *)&new_sstable->header.min_key, (unsigned char *)&new_sstable->header.min_key + sizeof(new_sstable->header.min_key));
	headerBinary.insert(headerBinary.end(), (unsigned char *)&new_sstable->header.max_key, (unsigned char *)&new_sstable->header.max_key + sizeof(new_sstable->header.max_key));

	// 将Header写入SSTable文件
	SSTable_file.write((char *)&headerBinary[0], headerBinary.size());

	// 创建SSTable的Bloom Filter的二进制序列
	std::vector<unsigned char> bloomFilterBinary = new_sstable->bloom_filter->get_binary_filter();

	// 将Bloom Filter写入SSTable文件
	SSTable_file.write((char *)&bloomFilterBinary[0], bloomFilterBinary.size());

	// 创建SSTable的元组的二进制序列
	std::vector<unsigned char> tupleBinary;

	// 依次将元组写入 SSTable 文件
	for (auto it = new_sstable->tuples.begin(); it != new_sstable->tuples.end(); it++)
	{
		// 将元组的 key, offset, vlen 拼接成二进制序列
		tupleBinary.insert(tupleBinary.end(), (unsigned char *)&it->key, (unsigned char *)&it->key + sizeof(it->key));
		tupleBinary.insert(tupleBinary.end(), (unsigned char *)&it->offset, (unsigned char *)&it->offset + sizeof(it->offset));
		tupleBinary.insert(tupleBinary.end(), (unsigned char *)&it->vlen, (unsigned char *)&it->vlen + sizeof(it->vlen));
	}

	// 将元组写入 SSTable 文件
	SSTable_file.write((char *)&tupleBinary[0], tupleBinary.size());

	// 关闭 SSTable 文件
	SSTable_file.close();

	// 最后将SSTable缓存到kvstore里，注意放在了缓存的第0层的第一个位置
	this->sstables[0].push_front(new_sstable);
}

// 对当前level层中all_sst_files中的字符串进行排序预处理，按照时间戳的大小进行排序、即时间戳大的代表新的
// 时间戳小的代表旧的，然后按照all_sst_files的从头至尾的顺序、依次把小时间戳的排在前面、大时间戳的排在后面
void KVStore::sort_all_sst_files_on_one_level(std::vector<std::string> &all_sst_files)
{
	std::sort(all_sst_files.begin(), all_sst_files.end(),
			  [](const std::string &a, const std::string &b)
			  {
				  // 提取出路径中的数字
				  std::string numStrA = a.substr(a.find_last_of('_') + 1, a.find_last_of('.') - a.find_last_of('_') - 1);
				  std::string numStrB = b.substr(b.find_last_of('_') + 1, b.find_last_of('.') - b.find_last_of('_') - 1);

				  // 将数字转换为整数
				  int numA = std::stoi(numStrA);
				  int numB = std::stoi(numStrB);

				  // 比较数字
				  return numA < numB;
			  });
}

/*
合并操作
若 Level 0 层中的文件数
量超过限制，则开始进行合并操作。对于 Level 0 层的合并操作来说，需要
将所有的Level 0 层中的 SSTable 与 Level 1 层中的部分 SSTable 进行合并，
随后将产生的新 SSTable 文件写入到 Level 1 层中。
具体方法如下：
1. 先统计 Level 0 层中所有 SSTable 所覆盖的键的区间。然后在 Level 1
层中找到与此区间有交集的所有 SSTable 文件。
2. 使用归并排序，将上述所有涉及到的 SSTable 进行合并，并将结果
每16 kB 分成一个新的 SSTable 文件（最后一个 SSTable 可以不足 16 kB），
写入到 Level 1 中。
3. 若产生的文件数超出 Level 1 层限定的数目，则从 Level 1 的 SSTable
中，优先选择时间戳最小的若干个文件（时间戳相等选择键最小的文件），
使得文件数满足层数要求，以同样的方法继续向下一层合并（若没有下一
层，则新建一层）。

从第0层开始，共16个level，每层存储的最大SSTable数是2^(level+1)个

记得合并完之后同时也要对缓存中的sstables进行相应的更新

一些注意事项：
1) 从 Level 1 层往下的合并开始，仅需将超出的文件往下一层进行合并
即可，无需合并该层所有文件。
2) 在合并时，如果遇到相同键 K 的多条记录，通过比较时间戳来决定
键 K 的最新值，时间戳大的记录被保留。
3) 完成一次合并操作之后需要更新涉及到的 SSTable 在内存中的缓存
信息。
4) 多个 SSTable 合并时，生成的 SSTable 时间戳为原 SSTable 中最大的
时间戳，因此生成的多个 SSTable 时间戳是可以相同的。
*/
// 这里采用的思路是：先在内存中进行合并操作，然后再将合并后的各层级的SSTable的结果按最大影响的层数依次进行写入硬盘
void KVStore::try_compaction()
{
	// 检查当前level_0层的sstables数量是否超出限制，如果没有超出限制则直接返回
	if (this->sstables[0].size() <= 2)
		return;

	// 如果超出限制，则开始进行合并操作
	// 因为这时 Level 0 层中的 SSTable 都已经缓存在了 kvstore->sstables[0] 中，所以直接从 kvstore->sstables[0] 中取出即可

	// 初始化变量
	std::deque<SSTable *> sstables_to_merge = this->sstables[0]; // 当前层的所有待合并的SSTable
	std::deque<SSTable *> related_sstables_at_next_level;		 // 下一层中与当前层中SSTable有交集的SSTable
	uint64_t max_level_during_compaction = 0;					 // 合并过程中涉及到的最大层数
	uint64_t max_time_stamp_at_current_compaction = 0;			 // 合并过程中涉及到的最大时间戳
	std::map<key_type, SSTableTuple> key_to_latest_tuple;		 // 用于存储合并过程中相同键的最新值的map
	uint64_t min_key_at_current_level = UINT64_MAX;				 // 当前层中的最小键
	uint64_t max_key_at_current_level = 0;						 // 当前层中的最大键

	// 同时、也需要清空Level_0中SSTable的缓存
	this->sstables[0].clear();

	// 从0层开始，循环合并缓存中的sstables
	for (uint64_t i = 0; i < sstable_max_level - 1; i++)
	{
		// 合并i层和i+1层 受影响的最高层数是i+1
		max_level_during_compaction = i + 1;

		// 初始化变量，每次合并都要重新初始化当前合并的最小键、最大键、最大时间戳
		min_key_at_current_level = UINT64_MAX;
		max_key_at_current_level = 0;
		max_time_stamp_at_current_compaction = 0;

		// 把在内存中的i层的sstables_to_merge按照时间戳升序排序，方便读入map
		std::sort(sstables_to_merge.begin(), sstables_to_merge.end(),
				  [](const SSTable *a, const SSTable *b)
				  {
					  return a->header.timestamp < b->header.timestamp;
				  });

		// 1. 先统计 Level i 层中所有 SSTable 所覆盖的键的区间。然后在 Level i + 1 层中找到与此区间有交集的所有 SSTable 缓存。
		// 首先、统计 Level i 层中所有 SSTable 所覆盖的键的区间
		for (auto it = sstables_to_merge.begin(); it != sstables_to_merge.end(); it++)
		{
			if ((*it)->header.min_key < min_key_at_current_level)
				min_key_at_current_level = (*it)->header.min_key;
			if ((*it)->header.max_key > max_key_at_current_level)
				max_key_at_current_level = (*it)->header.max_key;
			if ((*it)->header.timestamp > max_time_stamp_at_current_compaction)
				max_time_stamp_at_current_compaction = (*it)->header.timestamp;
		}

		// 然后、在 Level i + 1 层中找到与此区间有交集的所有 SSTable 缓存，将这些 SSTable 缓存放入 related_sstables_at_next_level 中，并从原缓存中删除
		// 从 Level 1 层往下的合并开始，仅需将超出的文件往下一层进行合并即可，无需合并该层所有文件
		if (i + 1 < sstable_max_level)
		{
			for (auto it = this->sstables[i + 1].begin(); it != this->sstables[i + 1].end(); it++)
			{
				if ((*it)->header.min_key <= max_key_at_current_level && (*it)->header.max_key >= min_key_at_current_level)
				{
					related_sstables_at_next_level.push_back(*it);
				}
			}
		}
		// 从原缓存中删除这些与此区间有交集的所有 SSTable 缓存
		for (auto it = related_sstables_at_next_level.begin(); it != related_sstables_at_next_level.end(); it++)
		{
			this->sstables[i + 1].erase(std::remove(this->sstables[i + 1].begin(), this->sstables[i + 1].end(), *it), this->sstables[i + 1].end());
		}
		// 把在内存中的i层的related_sstables_at_next_level按照时间戳升序排序，方便读入map
		std::sort(related_sstables_at_next_level.begin(), related_sstables_at_next_level.end(),
				  [](const SSTable *a, const SSTable *b)
				  {
					  return a->header.timestamp < b->header.timestamp;
				  });

		// 2. 使用map，将上述所有涉及到的 SSTable 进行合并，并将结果每16 kB 分成一个新的 SSTable 文件（最后一个 SSTable 可以不足 16 kB），写入到 Level 1 中。
		// 清空用于存储合并过程中相同键的最新值的map
		key_to_latest_tuple.clear();
		// 先遍历i层的related_sstables_at_next_level，将其中的tuple加入到map中
		for (auto it = related_sstables_at_next_level.begin(); it != related_sstables_at_next_level.end(); it++)
		{
			for (auto tuple_it = (*it)->tuples.begin(); tuple_it != (*it)->tuples.end(); tuple_it++)
			{
				SSTableTuple tuple = *tuple_it;
				key_to_latest_tuple[tuple.key] = tuple;
			}
		}
		// 再遍历i层的sstables_to_merge，将其中的tuple加入到map中
		for (auto it = sstables_to_merge.begin(); it != sstables_to_merge.end(); it++)
		{
			for (auto tuple_it = (*it)->tuples.begin(); tuple_it != (*it)->tuples.end(); tuple_it++)
			{
				SSTableTuple tuple = *tuple_it;
				key_to_latest_tuple[tuple.key] = tuple;
			}
		}
		// 遍历这个map 将其中的记录x写入新的sstables(缓存中) 每16kb一个文件
		std::vector<SSTableTuple> tuples_to_write;
		SSTableTuple tuple;
		uint64_t current_size = sstable_header_size + sstable_bloom_filter_size;
		for (auto it = key_to_latest_tuple.begin(); it != key_to_latest_tuple.end(); it++)
		{
			tuple = it->second;
			if (current_size + sstable_tuple_size > sstable_max_size)
			{
				// 创建新的SSTable缓存
				SSTable *new_sstable = new SSTable();
				new_sstable->header.timestamp = max_time_stamp_at_current_compaction;
				new_sstable->header.num = tuples_to_write.size();
				new_sstable->header.min_key = tuples_to_write[0].key;
				new_sstable->header.max_key = tuples_to_write[tuples_to_write.size() - 1].key;
				new_sstable->bloom_filter = new bloomFilter();
				for (auto tuple_it = tuples_to_write.begin(); tuple_it != tuples_to_write.end(); tuple_it++)
				{
					new_sstable->bloom_filter->insert(tuple_it->key);
					new_sstable->tuples.push_back(*tuple_it);
				}
				this->sstables[i + 1].push_front(new_sstable);
				tuples_to_write.clear();
				current_size = sstable_header_size + sstable_bloom_filter_size;
			}
			tuples_to_write.push_back(tuple);
			current_size += sstable_tuple_size;
		}
		// 将最后一个sst写入缓存
		SSTable *new_sstable = new SSTable();
		new_sstable->header.timestamp = max_time_stamp_at_current_compaction;
		new_sstable->header.num = tuples_to_write.size();
		new_sstable->header.min_key = tuples_to_write[0].key;
		new_sstable->header.max_key = tuples_to_write[tuples_to_write.size() - 1].key;
		new_sstable->bloom_filter = new bloomFilter();
		for (auto tuple_it = tuples_to_write.begin(); tuple_it != tuples_to_write.end(); tuple_it++)
		{
			new_sstable->bloom_filter->insert(tuple_it->key);
			new_sstable->tuples.push_back(*tuple_it);
		}
		this->sstables[i + 1].push_front(new_sstable);

		// 3. 若产生的文件数超出 Level i + 1 层限定的数目，则从 Level i + 1 的 SSTable 中，优先选择时间戳最小的若干个缓存
		// （时间戳相等选择键最小的文件），使得缓存数满足层数要求，以同样的方法继续向下一层合并（若没有下一层，则新建一层）。
		// 计算本层多余的文件数目
		int num_of_files_to_remove = this->sstables[i + 1].size() - (1 << (i + 2));
		if (num_of_files_to_remove <= 0)
			break;
		// 将第i层的sstables_to_merge，related_sstables_at_next_level清空
		sstables_to_merge.clear();
		related_sstables_at_next_level.clear();
		//  将level i+1层的sstables缓存按照时间戳降序排序、如果时间戳相等则按照键降序排序
		std::sort(this->sstables[i + 1].begin(), this->sstables[i + 1].end(),
				  [](const SSTable *a, const SSTable *b)
				  {
					  if (a->header.timestamp == b->header.timestamp)
						  return a->header.min_key > b->header.min_key;
					  return a->header.timestamp > b->header.timestamp;
				  });
		// 如果level i+1层的sstables缓存数量超出限制，则从level i+1层的sstables缓存中选择时间戳最小的若干个缓存加入sstables_to_merge
		// 选出时间戳最小、键最小的文件和下一层的文件合并
		for (int j = 0; j < num_of_files_to_remove; j++)
		{
			sstables_to_merge.push_back(this->sstables[i + 1].back());
			this->sstables[i + 1].pop_back();
		}
	}

	// std::cout << max_level_during_compaction << std::endl;

	// 最后再根据当前受影响的最大的层数、依次将合并后的各层级的SSTable的结果写入硬盘
	// 首先把受影响的0到最大层数的文件夹内的sst文件都删除
	for (uint64_t i = 0; i <= max_level_during_compaction; i++)
	{
		std::string level_dir = data_dir + "/Level_" + std::to_string(i);
		if (utils::dirExists(level_dir))
		{
			std::vector<std::string> all_sst_files;
			utils::scanDir(level_dir, all_sst_files);
			for (auto it = all_sst_files.begin(); it != all_sst_files.end(); it++)
			{
				std::string sst_path = level_dir + "/" + *it;
				utils::rmfile(sst_path.c_str());
			}
		}
	}
	// 接着、根据内存中缓存的sstables、依次将合并后的0到最大层数的层级的SSTable的缓存中的结果写入硬盘，只需要写SSTable，不需要写vLog
	for (uint64_t i = 0; i <= max_level_during_compaction; i++)
	{
		for (auto sst : sstables[i])
		{
			std::string sst_filepath = data_dir + "/Level_" + std::to_string(i) + "/SSTable_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "_" + std::to_string(sst->header.timestamp) + ".sst";
			std::ofstream sst_file(sst_filepath, std::ios::binary | std::ios::out | std::ios::app);
			// 准备Header的二进制序列
			std::vector<unsigned char> headerBinary;
			headerBinary.insert(headerBinary.end(), (unsigned char *)&sst->header.timestamp, (unsigned char *)&sst->header.timestamp + sizeof(sst->header.timestamp));
			headerBinary.insert(headerBinary.end(), (unsigned char *)&sst->header.num, (unsigned char *)&sst->header.num + sizeof(sst->header.num));
			headerBinary.insert(headerBinary.end(), (unsigned char *)&sst->header.min_key, (unsigned char *)&sst->header.min_key + sizeof(sst->header.min_key));
			headerBinary.insert(headerBinary.end(), (unsigned char *)&sst->header.max_key, (unsigned char *)&sst->header.max_key + sizeof(sst->header.max_key));
			// 将Header写入SSTable文件
			sst_file.write((char *)&headerBinary[0], headerBinary.size());
			// 准备Bloom Filter的二进制序列
			std::vector<unsigned char> bloomFilterBinary = sst->bloom_filter->get_binary_filter();
			// 将Bloom Filter写入SSTable文件
			sst_file.write((char *)&bloomFilterBinary[0], bloomFilterBinary.size());
			// 准备元组的二进制序列
			std::vector<unsigned char> tupleBinary;
			for (auto it = sst->tuples.begin(); it != sst->tuples.end(); it++)
			{
				// 将元组的 key, offset, vlen 拼接成二进制序列
				tupleBinary.insert(tupleBinary.end(), (unsigned char *)&it->key, (unsigned char *)&it->key + sizeof(it->key));
				tupleBinary.insert(tupleBinary.end(), (unsigned char *)&it->offset, (unsigned char *)&it->offset + sizeof(it->offset));
				tupleBinary.insert(tupleBinary.end(), (unsigned char *)&it->vlen, (unsigned char *)&it->vlen + sizeof(it->vlen));
			}
			// 将元组写入 SSTable 文件
			sst_file.write((char *)&tupleBinary[0], tupleBinary.size());
			// 关闭 SSTable 文件
			sst_file.close();
		}
	}
}

//--------------------------------------------------------------------------------
// 最后是为了测试写的两个不同的GET函数：
//--------------------------------------------------------------------------------

// get_from_disk不利用缓存、直接读硬盘中的SSTable进行查找、相当于每次查找的时候都要重新从磁盘里面加载一次缓存
std::string KVStore::get_from_disk(uint64_t key)
{
	// 先清空所有缓存的sstable
	for (int i = 0; i < sstable_max_level; i++)
	{
		this->sstables[i].clear();
	}
	// 遍历Level_0到sstable_max_level的目录
	for (int i = 0; i < sstable_max_level; i++)
	{
		std::string level_dir = data_dir + "/Level_" + std::to_string(i);
		if (!utils::dirExists(level_dir))
			utils::_mkdir(level_dir);
		else
		{
			// 如果有目录、那么将目录下的所有SSTable文件缓存到内存中，这里要求将时间戳最新的SSTable文件放在deque的最前面
			std::vector<std::string> all_sst_files;
			// 获取目录下的所有文件名，存入all_sst_files
			utils::scanDir(level_dir, all_sst_files);
			// 对当前level层中all_sst_files中的字符串进行排序预处理，按照时间戳的大小进行排序、即时间戳大的代表新的
			// 时间戳小的代表旧的，然后按照all_sst_files的从头至尾的顺序、依次把小时间戳的排在前面、大时间戳的排在后面
			sort_all_sst_files_on_one_level(all_sst_files);
			// 排序完了之后便对该level层的所有SSTable文件进行遍历和缓存
			for (auto it = all_sst_files.begin(); it != all_sst_files.end(); it++)
			{
				std::string sst_path = level_dir + "/" + *it;

				// 每次把读到的sst差到sstables当前层的deque的最前面，因为deque的最前面是时间戳最新的
				// 而all_sst_files中的文件是按照时间戳从小到大排列的、所以每次读到的文件都是尚未读完中的时间戳最小的
				SSTable *sst = new SSTable(sst_path);
				this->sstables[i].push_front(sst);
			}
		}
	}

	return get(key);
}

// get_without_bloom_filter不利用布隆过滤器、直接读取SSTable进行查找
std::string KVStore::get_without_bloom_filter(uint64_t key)
{
	// 首先从 MemTable 中进行查找，当查找到键 K 所对应的记录之后结束
	std::string result = memtable->get(key);

	// 如果result不为空，说明在MemTable中找到了
	if (result == delete_tag) // 如果找到的是删除标记，则说明该Key已被删除，返回空字符串
		return "";
	if (result != "") // 如果找到的是正常的值，则直接返回
		return result;

	// 如果在MemTable中没有找到，则从SSTable中查找，这里要从level_0到level_15依次查找。
	// 因为先查到的SSTable文件是时间戳最新的，相当于覆盖了时间戳旧的数据内容
	for (int i = 0; i < sstable_max_level; i++)
	{
		// 按照时间戳的降序对sstables进行排序
		std::sort(this->sstables[i].begin(), this->sstables[i].end(), [](SSTable *a, SSTable *b)
				  { return a->header.timestamp > b->header.timestamp; });
		// 从SSTable的缓存中查找key1到key2之间的键值对
		for (auto it = this->sstables[i].begin(); it != this->sstables[i].end(); it++)
		{
			// 这里不用 Bloom Filter 中判断 Key 是否在当前 SSTable 中
			// 如果可能存在则用二分查找在索引中找到对应的<key, offset, vlen>元组
			SSTableTuple tuple = (*it)->getTupleByBinarySearch(key);
			// 如果返回的元组、其三个值都是最大值，那么就是表示没找到的空元组、此时直接在下一个SSTable中寻找
			if (tuple.key == UINT64_MAX && tuple.offset == UINT64_MAX && tuple.vlen == UINT32_MAX)
				continue;
			// 如果找到了元组的key值
			if (tuple.key == key)
			{
				// 如果元组的vlen是0，说明该key已被删除，返回空字符串
				if (tuple.vlen == 0)
				{
					// std::cout << "check vlen == 0" << std::endl;
					return "";
				}

				// 之后即可从vLog中根据 offset 及 vlen 取出 value
				std::string value = vlog->getValue(tuple.offset, tuple.vlen);
				return value;
			}
		}
	}
	// 如果找遍了所有的层都没有这个 Key 值，则说明该 Key 不存在，返回空字符串
	return result;
}
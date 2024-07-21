#include "memtable.h"
#include <optional>
#include <ctime>

// 生成一个随机层数
int MemTable::randomLevel()
{
    int level = 1;
    while (static_cast<double>(std::rand()) / RAND_MAX < p && level <= maxLevel)
    {
        level++;
    }
    return level;
}

// 构造函数
MemTable::MemTable(double p) : p(p), maxLevel(32), currentLevel(1)
{
    header = new MemTableNode(maxLevel + 1, 0, "");
    size = sstable_header_size + sstable_bloom_filter_size;
}

// 析构函数
MemTable::~MemTable()
{
    MemTableNode *pt = header;
    while (pt)
    {
        MemTableNode *qt = pt->forward[1];
        delete pt;
        pt = qt;
    }
}

// 将key和value插入跳表中、如果key已经存在则更新value
void MemTable::put(key_type key, const value_type &val)
{
    std::vector<MemTableNode *> update(maxLevel + 1, nullptr);
    MemTableNode *x = header;
    for (int i = currentLevel; i >= 1; i--)
    {
        while (x->forward[i] != nullptr && x->forward[i]->key < key)
            x = x->forward[i];
        update[i] = x;
    }
    x = x->forward[1];
    if (x != nullptr && x->key == key)
        x->value = val;
    else
    {
        int level = randomLevel();
        if (level > currentLevel)
        {
            for (int i = currentLevel + 1; i <= level; i++)
                update[i] = header;
            currentLevel = std::min(level, maxLevel);
        }
        x = new MemTableNode(level, key, val);
        for (int i = 1; i <= level; i++)
        {
            x->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = x;
        }
        // 增加size，size的大小是生成的SSTable的一个Tuple的大小
        size += sizeof(SSTableTuple::key) + sizeof(SSTableTuple::offset) + sizeof(SSTableTuple::vlen);
    }
}

// 获取key对应的value，如果key不存在则返回空
std::string MemTable::get(key_type key) const
{
    MemTableNode *x = header;
    for (int i = currentLevel; i >= 1; i--)
    {
        while (x->forward[i] != nullptr && x->forward[i]->key < key)
            x = x->forward[i];
    }
    x = x->forward[1];
    if (x != nullptr && x->key == key)
        return x->value;
    else
        return "";
}

// 初始化 先清空跳表 再重新初始化head
void MemTable::reset()
{
    MemTableNode *pt = header;
    while (pt)
    {
        MemTableNode *qt = pt->forward[1];
        delete pt;
        pt = qt;
    }
    header = new MemTableNode(maxLevel + 1, 0, "");
    currentLevel = 1;
    size = sstable_header_size + sstable_bloom_filter_size;
}

// 扫描key在[K1, K2]范围内的所有键值对，存入scan_map中
// 跳过被删除的节点，删除被删除的节点 可能来自sstable的结果 因为内存是最优先的
void MemTable::scan(key_type K1, key_type K2, std::map<uint64_t, std::string> &scan_map)
{
    MemTableNode *x = header;
    for (int i = currentLevel; i >= 1; i--)
    {
        while (x->forward[i] != nullptr && x->forward[i]->key < K1)
            x = x->forward[i];
    }
    // 从K1开始扫描
    x = x->forward[1];
    while (x != nullptr && x->key <= K2)
    {
        if (x->value != delete_tag)
            scan_map[x->key] = x->value;
        x = x->forward[1];
    }
}

// 获取跳表的大小
size_t MemTable::getSize() const
{
    return size;
}

// 获取跳表的所有键值对
std::vector<std::pair<key_type, value_type>> MemTable::getAll() const
{
    std::vector<std::pair<key_type, value_type>> res;
    MemTableNode *x = header->forward[1];
    while (x != nullptr)
    {
        res.push_back({x->key, x->value});
        x = x->forward[1];
    }
    return res;
}

// 判断跳表是否为空
bool MemTable::isEmpty() const
{
    return header->forward[1] == nullptr;
}
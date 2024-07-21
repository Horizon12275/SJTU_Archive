// implement a radix tree that support node compressino and store int32_t values
// each parent node has 4 children, representing 2 bits
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <stack>
#include "tree.hpp"

class CompressedRadixTreeNode
{
public:
    CompressedRadixTreeNode *children[4]; // 00, 01, 10, 11作为索引值
    int32_t value; // 存储的可能并不是00，01，10，11，而是一个若干个00，01，10，11组合的值（转化为int存储）
    uint32_t length; // 存储值的有效位数

    CompressedRadixTreeNode()
    {
        for (int i = 0; i < 4; ++i)
        {
            children[i] = nullptr;
        }
    }
};

class CompressedRadixTree : public Tree
{
private:
    CompressedRadixTreeNode *root;
    void deleteNode(CompressedRadixTreeNode *node);
    uint32_t getMaxDepth(CompressedRadixTreeNode *node);

public : CompressedRadixTree();
    ~CompressedRadixTree();
    // basic operation
    void insert(int32_t value);
    void remove(int32_t value);
    bool find(int32_t value);
    // statistics
    uint32_t size();
    uint32_t height();
    // debug
    void print();
};
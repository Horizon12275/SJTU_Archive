// implement a radix tree that support node compressino and store int32_t values
// each parent node has 4 children, representing 2 bits
#include <cstdint>
#include <algorithm>
#include <stack>
#include <iostream>
#include <unordered_map>
#include "tree.hpp"

class CompressedRadixTreeNode
{
public:
    std::unordered_map<int, CompressedRadixTreeNode *> children; // Each node has any amount of children
    int32_t value;
    uint32_t length;
    uint32_t height;

    CompressedRadixTreeNode()
    {
        value = 0;
        length = 0;
        height = 0;
    }

};

class CompressedRadixTree : public Tree
{
private:
    CompressedRadixTreeNode *root;
    uint32_t _size;
    uint32_t _height;
    void deleteNode(CompressedRadixTreeNode *node);
public:
    CompressedRadixTree();
    ~CompressedRadixTree();
    // basic operation
    void insert(int32_t value);
    void remove(int32_t value);
    bool find(int32_t value);
    // statistics
    uint32_t size();
    uint32_t height();
};
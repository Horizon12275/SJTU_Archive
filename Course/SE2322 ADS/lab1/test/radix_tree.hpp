#include <stack>
#include <cstdint>
#include "tree.hpp"

class RadixTreeNode
{
public:
    RadixTreeNode *children[4]; // Each node has 4 children
    bool isLeaf;

    RadixTreeNode()
    {
        for (int i = 0; i < 4; ++i)
        {
            children[i] = nullptr;
        }
        isLeaf = false;
    }
};

class RadixTree : public Tree
{
private:
    RadixTreeNode *root;
    uint32_t _size;
    uint32_t _height;
    void deleteNode(RadixTreeNode *node);

public:
    RadixTree();
    ~RadixTree();
    // basic operation
    void insert(int32_t value);
    void remove(int32_t value);
    bool find(int32_t value);
    // statistics
    uint32_t size();
    uint32_t height();
};

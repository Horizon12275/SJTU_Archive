#include "radix_tree.hpp"

RadixTree::RadixTree()
{
    root = new RadixTreeNode();
    _size = 1;
}

RadixTree::~RadixTree()
{
    deleteNode(root);
}

void RadixTree::deleteNode(RadixTreeNode *node)
{
    if (node)
    {
        for (int i = 0; i < 4; ++i)
            deleteNode(node->children[i]);
        delete node;
    }
}

void RadixTree::insert(int32_t value)
{
    RadixTreeNode *current = root;
    for (int i = 30; i >= 0; i -= 2)
    {
        int index = ((value >> i) & 3); // Get two bits at a time
        if (!current->children[index]){
            current->children[index] = new RadixTreeNode();
            _size++;
        }
        current = current->children[index];
    }
    current->isLeaf = true;
}


void RadixTree::remove(int32_t value)
{
    RadixTreeNode *current = root;
    std::stack<RadixTreeNode *> path;
    for (int i = 30; i >= 0; i -= 2)
    {
        int index = ((value >> i) & 3);
        if (!current->children[index])
            return;
        current = current->children[index];
        path.push(current);
    }
    // delete the leaf node, and check the path, if the node in the path has no other children, delete it recursively
    int i = 0;
    while(!path.empty())
    {
        // get the node
        RadixTreeNode *node = path.top();
        path.pop();

        // edit the parent node
        RadixTreeNode *parent = path.empty() ? root : path.top();
        parent->children[(value >> i) & 3] = nullptr;

        // delete the node
        delete node;
        _size--;

        // check the path
        bool hasChild = false;
        for (int i = 0; i < 4; ++i)
        {
            if (parent->children[i])
            {
                hasChild = true;
                break;
            }
        }
        
        // if the node has no other children, delete it
        if (hasChild)
            break;
        i += 2;
    }
}

bool RadixTree::find(int32_t value)
{
    RadixTreeNode *current = root;
    for (int i = 30; i >= 0; i -= 2)
    {
        int index = ((value >> i) & 3);
        if (!current->children[index])
            return false;
        current = current->children[index];
    }
    return current->isLeaf;
}

uint32_t RadixTree::size()
{
    return _size;
}

uint32_t RadixTree::height()
{
    return _size == 1 ? 1 : 17;
}

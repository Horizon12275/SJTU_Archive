#include "compressed_radix_tree.hpp"

CompressedRadixTree::CompressedRadixTree()
{
    root = new CompressedRadixTreeNode();
    root->value = 0;
    root->length = 0;
    root->height = 1;
    _size = 1;
    _height = 1;
}

CompressedRadixTree::~CompressedRadixTree()
{
    deleteNode(root);
}

void CompressedRadixTree::deleteNode(CompressedRadixTreeNode *node)
{
    if (node)
    {
        for (auto it = node->children.begin(); it != node->children.end(); ++it)
            deleteNode(it->second);
        delete node;
    }
}

void CompressedRadixTree::insert(int32_t value)
{
    CompressedRadixTreeNode *current = root;
    int32_t lengthSum = 0;
    while (lengthSum < 8)
    {
        bool ok_find = false;
        if (current != nullptr) {
            for (auto it = current->children.begin(); it != current->children.end(); ++it)
            {
                int32_t currentValue = it->first;
                uint32_t currentLength = it->second->length;
                lengthSum += currentLength;
                if (currentValue == ((value >> (8 - lengthSum)) & ((1 << currentLength) - 1)))
                {
                    current = it->second;
                    ok_find = true;
                    break;
                }
                lengthSum -= currentLength;
            }
        }
        if (!ok_find){
            CompressedRadixTreeNode *newNode = new CompressedRadixTreeNode();
            newNode->value = (value & ((1 << (8 - lengthSum)) - 1));
            newNode->length = 8 - lengthSum;
            newNode->height = current->height + 1;
            current->children[newNode->value] = newNode;
            _size++;
            _height = std::max(_height, newNode->height);
            break;
        }
    }
}

void CompressedRadixTree::remove(int32_t value)
{
    
}

bool CompressedRadixTree::find(int32_t value)
{
    CompressedRadixTreeNode *current = root;
    int32_t lengthSum = 0;
    while(lengthSum < 8){
        bool ok_find = false;
        if (current != nullptr){
            for (auto it = current->children.begin(); it != current->children.end(); ++it)
            {
                int32_t currentValue = it->first;
                int32_t currentLength = it->second->length;
                lengthSum += currentLength;
                if (currentValue == ((value >> (8 - lengthSum)) & ((1 << currentLength) - 1)))
                {
                    current = it->second;
                    ok_find = true;
                    break;
                }
                lengthSum -= currentLength;
            }
        }
        if (!ok_find)
            return false;
    }
    return true;
}

uint32_t CompressedRadixTree::size()
{
    return _size;
}

uint32_t CompressedRadixTree::height()
{
    return _height;
}

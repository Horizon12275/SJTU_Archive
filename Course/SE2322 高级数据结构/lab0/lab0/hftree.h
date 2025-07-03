#ifndef HFTREE_H
#define HFTREE_H

#include <string>
#include <map>
#include <queue>
#include <memory>
#include <iostream>
#include <set>

class hfTree{
private:
    struct hfTreeNode{ //define the structure of the node
        std::string data;
        int weight;
        hfTreeNode *left;
        hfTreeNode *right;
        hfTreeNode(std::string data,int weight) : data(data), weight(weight), left(nullptr), right(nullptr) {}
    };
    hfTreeNode *root; //the root of the tree
    std::map<std::string, std::string> codingTable;

public:
    enum class Option
    {
        SingleChar,
        MultiChar
    };
    void generatecodingtable(const hfTreeNode *node, std::string code);
    ~hfTree();
    void deleteNode(hfTreeNode *node);
    void singlebuild(const std::string &text);
    void multibuild(const std::string &text);
    hfTree(const std::string &text, const Option op);
    std::map<std::string, std::string> getCodingTable();
};

#endif
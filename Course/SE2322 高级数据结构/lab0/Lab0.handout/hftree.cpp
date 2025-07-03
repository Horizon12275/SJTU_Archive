#include "hftree.h"

hfTree::hfTree(const std::string &text, const Option op){
    switch (op){
    case Option::SingleChar:
        singlebuild(text);
        break;
    case Option::MultiChar:
        multibuild(text);
        break;
    default:
        break;
    }
}

void hfTree::generatecodingtable(const hfTreeNode *node, std::string code){   
    if(node == nullptr) return;
    if (node->left == nullptr && node->right == nullptr){
        codingTable[node->data] = code;
        return;
    } // dfs to generate coding table
    generatecodingtable(node->left, code + "0");
    generatecodingtable(node->right, code + "1");
}

void hfTree::singlebuild(const std::string &text){
    std::map<std::string, int> freqMap;    // Build frequency map
    for (int i = 0; i < text.size(); i++){
        std::string key = text.substr(i, 1);
        freqMap[key]++;
    }
    struct cmp{ // Custom comparator for priority queue
        bool operator()(hfTreeNode *a, hfTreeNode *b){
            if (a->weight == b->weight)
                return a->data > b->data;
            else
                return a->weight > b->weight;
        }
    };
    std::priority_queue<hfTreeNode *, std::vector<hfTreeNode *>, cmp> nodeQueue;
    for (const auto &pair : freqMap){
        nodeQueue.push(new hfTreeNode(pair.first, pair.second));
    }
    while (nodeQueue.size() > 1){ // Build huffman tree
        hfTreeNode *left = nodeQueue.top();
        nodeQueue.pop();
        hfTreeNode *right = nodeQueue.top();
        nodeQueue.pop();
        hfTreeNode *parent = new hfTreeNode(min(left->data, right->data), left->weight + right->weight);
        parent->left = left;
        parent->right = right;
        nodeQueue.push(parent);
    }
    root = nodeQueue.top();
    generatecodingtable(root, ""); // Generate coding table
}

void hfTree::multibuild(const std::string &text){
    std::map<std::string, int> multiCharFreqMap; // consider multi-char
    for (int i = 0; i < text.size() - 1; i++){
        std::string key = text.substr(i, 2);
        multiCharFreqMap[key]++;
    }
    struct cmp4Multi{
        bool operator()(hfTreeNode *a, hfTreeNode *b){
            if (a->weight == b->weight)
                return a->data > b->data;
            else
                return a->weight < b->weight;
        }
    };
    std::priority_queue<hfTreeNode *, std::vector<hfTreeNode *>, cmp4Multi> multiQueue;
    for (const auto &pair : multiCharFreqMap){
        if (pair.second >= 1)
            multiQueue.push(new hfTreeNode(pair.first, pair.second));
    }
    std::set<std::string> multiCharSet; // Get the top 3 most frequent multi-char
    int cnt = 100;
    while (multiQueue.size() >= 1 && cnt >= 1){
        std::string multiNode = multiQueue.top()->data;
        multiQueue.pop();
        multiCharSet.insert(multiNode);
        cnt--;
    }
    // Build frequency map
    std::map<std::string, int> freqMap;
    for (int i = 0; i < text.size(); i++){
        if(i < text.size() - 1){
            std::string key = text.substr(i, 2);
            if(multiCharSet.find(key) != multiCharSet.end()){
                freqMap[key]++;
                i++;
                continue;
            }
        }
        std::string key = text.substr(i, 1);
        freqMap[key]++;
    }
    /* ... */

    struct cmp
    {
        bool operator()(hfTreeNode *a, hfTreeNode *b)
        {
            if (a->weight == b->weight)
                return a->data > b->data;
            else
                return a->weight > b->weight;
        }
    };

    std::priority_queue<hfTreeNode *, std::vector<hfTreeNode *>, cmp> nodeQueue;
    for (const auto &pair : freqMap)
    {
        if (pair.second >= 1)
            nodeQueue.push(new hfTreeNode(pair.first, pair.second));
    }

    while (nodeQueue.size() > 1)
    {
        hfTreeNode *left = nodeQueue.top();
        nodeQueue.pop();
        hfTreeNode *right = nodeQueue.top();
        nodeQueue.pop();
        hfTreeNode *parent = new hfTreeNode(min(left->data,right->data), left->weight + right->weight);
        parent->left = left;
        parent->right = right;
        nodeQueue.push(parent);
    }

    root = nodeQueue.top();
}

hfTree::~hfTree(){
    deleteNode(root);
}

void hfTree::deleteNode(hfTreeNode *node){
    if (node == nullptr)
        return;
    deleteNode(node->left);
    deleteNode(node->right);
    delete node;
}

std::map<std::string, std::string> hfTree::getCodingTable(){
    generatecodingtable(root, "");
    return codingTable;
}
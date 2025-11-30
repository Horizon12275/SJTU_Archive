#include "compressed_radix_tree.hpp"

CompressedRadixTree::CompressedRadixTree()
{
    root = new CompressedRadixTreeNode();
    root->value = 0;
    root->length = 0;
}

CompressedRadixTree::~CompressedRadixTree()
{
    deleteNode(root);
}

void CompressedRadixTree::deleteNode(CompressedRadixTreeNode *node)
{
    if (node)
    {
        for (int i = 0; i < 4; ++i)
            deleteNode(node->children[i]);
        delete node;
    }
}

void CompressedRadixTree::insert(int32_t value)
{
    CompressedRadixTreeNode *current = root;

    for (int32_t i = 30; i >= 0;)
    {
        // 索引位
        int index = ((value >> i) & 3);
        // 如果没有这个节点，创建一个新的节点
        if (!current->children[index])
        {
            current->children[index] = new CompressedRadixTreeNode();
            // 取value的后i+2位作为当前孩子节点的value
            if (i + 2 == 32) // 特判32位的情况，防止溢出
                current->children[index]->value = value;
            else
                current->children[index]->value = (value & ((1 << (i + 2)) - 1));
            current->children[index]->length = i + 2;
            return;
        }
        // 得到节点的比较长度
        int current_child_length = current->children[index]->length;
        // 得到value的比较部分
        int32_t value_cmp_part;
        if (current_child_length == 32) // 特判32位的情况，防止溢出
            value_cmp_part = value;
        else // 得到value的前current_child_length位
            value_cmp_part = (value >> (i + 2 - current_child_length)) & ((1 << current_child_length) - 1);
        int32_t child_value = current->children[index]->value;
        // 如果不相等，需要分裂节点
        // value_cmp_part和child_value的length是相等的
        if (value_cmp_part != child_value)
        {
            // 如果child_value当前的值是01，那么child的length为2，如果
            // 再找出value_cmp_part和child_value的共同前缀，直到遇到不同的位
            int32_t common_prefix = 0;
            while (common_prefix < current_child_length && ((value_cmp_part >> (current_child_length - common_prefix - 1)) & 1) == ((child_value >> (current_child_length - common_prefix - 1)) & 1))
                common_prefix++;
            if (common_prefix % 2 == 1)
                common_prefix--;
            if (common_prefix == 0)
                return;
            // 最后这个公共前缀还需要减去current_child_length的偏移量
            // common_prefix = common_prefix - 32 + current_child_length;
            // 创建一个新的节点，存储共同前缀
            CompressedRadixTreeNode *new_node = new CompressedRadixTreeNode();
            // 取value_cmp_part的前common_prefix位作为新节点的value
            new_node->value = (value_cmp_part >> (current_child_length - common_prefix)) & ((1 << common_prefix) - 1);
            new_node->length = common_prefix;
            // 创建一个新的节点，存储value的剩余部分
            CompressedRadixTreeNode *new_node2 = new CompressedRadixTreeNode();
            // 取value的剩余部分作为新节点的value
            new_node2->value = (value & ((1 << (i + 2 - common_prefix)) - 1));
            // value的长度是32减去已判断完的部分，再减去current_child_length的偏移量
            new_node2->length = 32 - (30 - i) - common_prefix;
            // 创建一个新的节点，存储child_value的剩余部分
            CompressedRadixTreeNode *new_node3 = new CompressedRadixTreeNode();
            // 取child_value的后i+2-common_prefix位作为新节点的value
            new_node3->value = (child_value & ((1 << (current_child_length - common_prefix)) - 1));
            new_node3->length = current_child_length - common_prefix;
            // 将原节点的孩子变成new_node3的孩子
            for (int j = 0; j < 4; ++j)
            {
                new_node3->children[j] = current->children[index]->children[j];
                current->children[index]->children[j] = nullptr;
            }
            // 找出new_node2的前2位作为索引值，插入new_node的对应孩子位
            int new_node2_index = (new_node2->value >> (i + 2 - common_prefix - 2)) & 3;
            new_node->children[new_node2_index] = new_node2;
            // 找出new_node3的前2位作为索引值，插入new_node的对应孩子位
            int new_node3_index = (new_node3->value >> (current_child_length - common_prefix - 2)) & 3;
            new_node->children[new_node3_index] = new_node3;
            // 用tmp释放原节点的空间
            CompressedRadixTreeNode *tmp = current->children[index];
            delete tmp;
            // 将new_node插入current的孩子位
            current->children[index] = new_node;
            return;
        }
        // 移动到下一个节点
        i -= current_child_length;
        current = current->children[index];
    }
}

void CompressedRadixTree::remove(int32_t value)
{
    CompressedRadixTreeNode *current = root;
    std::stack<CompressedRadixTreeNode *> path;
    for (int32_t i = 30; i >= 0;)
    {
        // 索引位
        int index = ((value >> i) & 3);
        // 如果没有这个节点，返回
        if (!current->children[index])
            return;
        // 得到节点的比较长度
        uint32_t current_child_length = current->children[index]->length;
        // 得到value的比较部分
        int32_t value_cmp_part;
        if (current_child_length == 32) // 特判32位的情况，防止溢出
            value_cmp_part = value;
        else
            value_cmp_part = (value >> (i + 2 - current_child_length)) & ((1 << current_child_length) - 1);
        // 如果不相等，返回
        if (value_cmp_part != current->children[index]->value)
            return;
        // 移动到下一个节点
        i -= current_child_length;
        current = current->children[index];
        path.push(current);
    }
    // 在删除叶⼦节点后，若发现其⽗节点的⼦节点数变为1，需要将其与其⼦节点进⾏合并
    while (!path.empty())
    {
        CompressedRadixTreeNode *node = path.top();
        path.pop();
        CompressedRadixTreeNode *parent = path.empty() ? root : path.top();
        int i = 0;
        for (int j = 0; j < 4; ++j)
        {
            if (parent->children[j] == node)
            {
                i = j;
                break;
            }
        }
        parent->children[i] = nullptr;
        delete node;
        int child_count = 0;
        for (int j = 0; j < 4; ++j)
        {
            if (parent->children[j])
            {
                child_count++;
                
            }
        }
        if(child_count == 1){
            CompressedRadixTreeNode *child = nullptr;
            for (int j = 0; j < 4; ++j)
            {
                if (parent->children[j])
                {
                    child = parent->children[j];
                    break;
                }
            }
            //将子节点拼接在父节点后，并删除子节点
            parent->value = (parent->value << child->length) | child->value;
            parent->length += child->length;
            
            for (int j = 0; j < 4; ++j)
            {
                parent->children[j] = child->children[j];
                child->children[j] = nullptr;
            }
            delete child;
            break;
        }
        if(child_count > 1)
            break;
    }
}

bool CompressedRadixTree::find(int32_t value)
{
    CompressedRadixTreeNode *current = root;
    for (int32_t i = 30; i >= 0;)
    {
        // 索引位
        int index = ((value >> i) & 3);
        // 如果没有这个节点，返回false
        if (!current->children[index])
            return false;
        // 得到节点的比较长度
        uint32_t current_child_length = current->children[index]->length;
        // 得到value的比较部分
        int32_t value_cmp_part;
        if (current_child_length == 32) // 特判32位的情况，防止溢出
            value_cmp_part = value;
        else
            value_cmp_part = (value >> (i + 2 - current_child_length)) & ((1 << current_child_length) - 1);
        // 如果不相等，返回false
        if (value_cmp_part != current->children[index]->value)
            return false;
        // 移动到下一个节点
        i -= current_child_length;
        current = current->children[index];
    }
    return true;
}

uint32_t CompressedRadixTree::size()
{
    //从根节点遍历所有节点，统计节点数
    uint32_t count = 0;
    std::stack<CompressedRadixTreeNode *> s;
    s.push(root);
    while (!s.empty())
    {
        CompressedRadixTreeNode *current = s.top();
        s.pop();
        count++;
        for (int i = 0; i < 4; ++i)
        {
            if (current->children[i])
                s.push(current->children[i]);
        }
    }
    return count;
}

uint32_t CompressedRadixTree::height()
{
    return getMaxDepth(root);
}

uint32_t CompressedRadixTree::getMaxDepth(CompressedRadixTreeNode *node)
{
    if (node == nullptr)
    {
        return 0;
    }

    uint32_t maxDepth = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (node->children[i] != nullptr)
        {
            uint32_t childDepth = getMaxDepth(node->children[i]);
            if (childDepth > maxDepth)
            {
                maxDepth = childDepth;
            }
        }
    }
    return maxDepth + 1; // Add 1 to account for the current node
}

void CompressedRadixTree::print()
{
    std::cout << "CompressedRadixTree" << std::endl;
    std::cout << "size: " << size() << std::endl;
    std::cout << "height: " << height() << std::endl;
    std::cout << "root: " << root << std::endl;
    std::cout << "root->value: " << root->value << std::endl;
    std::cout << "root->length: " << root->length << std::endl;
    for (int i = 0; i < 4; ++i)
    {
        std::cout << "root->children[" << i << "]: " << root->children[i] << std::endl;
        if (root->children[i])
        {
            std::cout << "root->children[" << i << "]->value: " << root->children[i]->value << std::endl;
            std::cout << "root->children[" << i << "]->length: " << root->children[i]->length << std::endl;
        }
        std::cout << std::endl;
    }
    // 打印所有节点的信息
    std::stack<CompressedRadixTreeNode *> s;
    s.push(root);
    while (!s.empty())
    {
        CompressedRadixTreeNode *current = s.top();
        s.pop();
        for (int i = 0; i < 4; ++i)
        {
            if (current->children[i])
            {
                s.push(current->children[i]);
                std::cout << "current->children[" << i << "]: " << current->children[i] << std::endl;
                std::cout << "current->children[" << i << "]->value: " << current->children[i]->value << std::endl;
                std::cout << "current->children[" << i << "]->length: " << current->children[i]->length << std::endl;
                std::cout << std::endl;
            }
        }
    }
}
#include "radix_tree.hpp"
#include "compressed_radix_tree.hpp"
#include <cstring>
#include <fstream>
#include <iostream>

// input file format:
// n
// op1 v1
// ...
// opn vn

// output file format:
// result1
// ...
// resultn
void testTree(const char *input, const char *output, TreeType treeType)
{
    Tree *tree;
    switch (treeType)
    {
    case TreeType::RadixTree:
        tree = new class RadixTree();
        break;
    case TreeType::CompressedRadixTree:
        tree = new class CompressedRadixTree();
        break;
    default:
        break;
    }
    std::ifstream ifs(input);
    std::ofstream ofs(output);
    int n;
    ifs >> n;
    for (int i = 0; i < n; i++)
    {
        char op[10];
        int value;
        ifs >> op;
        if (strcmp(op, "print") != 0)
        {
            ifs >> value;
        }
        if (strcmp(op, "insert") == 0)
        {
            tree->insert(value);
        }
        else if (strcmp(op, "remove") == 0)
        {
            tree->remove(value);
        }
        else if (strcmp(op, "find") == 0)
        {
            ofs << (tree->find(value) ? "True" : "False") << std::endl;
        }
        else if (strcmp(op, "print") == 0)
        {
            ofs << tree->size() << " " << tree->height() << std::endl;
        }
    }
}

// argv[1]: input file path
// argv[2]: output file path
// argv[3]: tree type
int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cout << "[usage]: ./main [input_file_path] [output_file_path] [tree_type]" << std::endl;
        return 0;
    }
    TreeType treeType;
    if (strcmp(argv[3], "RadixTree") == 0)
    {
        treeType = TreeType::RadixTree;
    }
    else if (strcmp(argv[3], "CompressedRadixTree") == 0)
    {
        treeType = TreeType::CompressedRadixTree;
    }
    testTree(argv[1], argv[2], treeType);
    return 0;
}

// #include "radix_tree.hpp"
// #include "compressed_radix_tree.hpp"
// #include <cstring>
// #include <fstream>
// #include <iostream>

// int main(int argc, char **argv)
// {
//     Tree *tree;
//     tree = new class CompressedRadixTree();

//     tree->insert(1);
//     tree->insert(2);
//     tree->insert(3);
//     tree->insert(4);
//     tree->insert(5);
//     tree->insert(6);
//     tree->insert(7);
//     tree->insert(8);
//     tree->insert(9);
//     tree->insert(10);
//     tree->remove(1);
//     tree->remove(2);
//     tree->remove(3);
//     tree->remove(4);
//     tree->remove(5);
//     tree->remove(6);
//     tree->remove(7);
//     tree->remove(8);
//     tree->remove(9);
//     tree->remove(10);



//     delete tree;
//     return 0;
// }

// // test
// //  std::cout << "CompressedRadixTree" << std::endl;
// //  tree->insert(1);
// //  std::cout << tree->find(2) << std::endl;
// //  tree->insert(2);
// //  std::cout << tree->find(2) << std::endl;
// //  tree->insert(3);
// //  tree->insert(4);
// //  std::cout << tree->find(123123) << std::endl;
// //  tree->insert(123123);
// //  tree->insert(3);
// //  tree->insert(0);
// //  std::cout << tree->find(1) << std::endl;
// //  std::cout << tree->find(2) << std::endl;
// //  std::cout << tree->find(123123)<<std::endl;
// //  std::cout << tree->find(0)<<std::endl;
// //  std::cout << tree->find(5)<<std::endl;
// //  //tree->remove(123123);

// // Test Case 1: Insert and find a single element
// tree->insert(5);
// // bool found1 = tree->find(5);
// // std::cout << "Element 5 found: " << found1 << std::endl;

// // Test Case 2: Insert multiple elements and find them
// tree->insert(10);
// tree->insert(15);
// tree->insert(20);
// // bool found2 = tree->find(10);
// // bool found3 = tree->find(15);
// // bool found4 = tree->find(20);
// // std::cout << "Element 10 found: " << found2 << std::endl;
// // std::cout << "Element 15 found: " << found3 << std::endl;
// // std::cout << "Element 20 found: " << found4 << std::endl;

// // Test Case 3: Insert duplicate elements and find them
// tree->insert(25);
// tree->insert(25);

// bool found5 = tree->find(25);
// std::cout << "Element 25 found: " << found5 << std::endl;

// tree->remove(25);

// found5 = tree->find(25);
// std::cout << "Element 25 found: " << found5 << std::endl;

// tree->remove(25);

// tree->remove(20);
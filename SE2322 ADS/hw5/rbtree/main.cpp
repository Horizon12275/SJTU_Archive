#include <iostream>
#include <algorithm>
#include <random>
#include "rbtree.h"
int main() {
    RedBlackTree tree_ordered;

    // RedBlackTree tree;
    // tree.insert(7);
    // tree.insert(3);
    // tree.insert(18);
    // tree.insert(10);
    // tree.insert(22);
    // tree.insert(8);
    // tree.insert(11);
    // tree.insert(26);
    // std::cout << "Inorder traversal of the constructed tree: \n";
    // tree.inorder();
    // tree.printStatistics();
    // std::cout << std::endl;

    // 创建包含0到9999的数组
    int arr[10000];
    for (int i = 0; i < 10000; ++i){
        arr[i] = i;
        tree_ordered.insert(i);
    }

    tree_ordered.printStatistics();
    
    // 使用随机数引擎来打乱数组顺序
    std::random_device rd;
    std::mt19937 g(rd());

    // n为循环执行次数
    int n = 10000;

    // 统计所有棵树的fixViolation_times，rotateLeft_times，rotateRight_times，re_color_times
    int fixViolation_times = 0;
    int rotateLeft_times = 0;
    int rotateRight_times = 0;
    int re_color_times = 0;

    // 创建一个新的树，插入打乱顺序的数组，并统计fixViolation_times，rotateLeft_times，rotateRight_times，re_color_times
    for (int i = 0; i < n; ++i){
        RedBlackTree tree_shuffled;
        std::shuffle(std::begin(arr), std::end(arr), g);
        for (int j = 0; j < 10000; ++j){
            tree_shuffled.insert(arr[j]);
        }
        fixViolation_times += tree_shuffled.getFixViolationTimes();
        rotateLeft_times += tree_shuffled.getRotateLeftTimes();
        rotateRight_times += tree_shuffled.getRotateRightTimes();
        re_color_times += tree_shuffled.getReColorTimes();
    }

    // 求所有树的fixViolation_times，rotateLeft_times，rotateRight_times，re_color_times的平均值
    fixViolation_times /= n;
    rotateLeft_times /= n;
    rotateRight_times /= n;
    re_color_times /= n;

    // 打印所有树的统计信息
    std::cout << std::endl << "RedBlackTree_shuffled Statistics: " << std::endl;
    std::cout << "Fix Violation: " << fixViolation_times << std::endl;
    std::cout << "Re-color: " << re_color_times << std::endl;
    std::cout << "Rotate Left: " << rotateLeft_times << std::endl;
    std::cout << "Rotate Right: " << rotateRight_times << std::endl;
    std::cout << "Rotate: " << rotateLeft_times + rotateRight_times << std::endl;

    return 0;
}
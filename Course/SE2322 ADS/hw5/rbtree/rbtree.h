#ifndef RBTREE_H
#define RBTREE_H

enum Color { RED, BLACK };

struct Node {
    int data;
    Color color;
    Node *parent, *left, *right;

    Node(int data);
};

class RedBlackTree {
private:
    Node* root;
    int fixViolation_times;
    int rotateLeft_times;
    int rotateRight_times;
    int re_color_times;

    void rotateLeft(Node* x);
    void rotateRight(Node* x);
    void fixViolation(Node* pt);
    Node* BSTInsert(Node* root, Node* pt);
    void inorderUtil(Node* root);

public:
    RedBlackTree();
    void insert(const int data);
    void inorder();
    void printStatistics();
    int getFixViolationTimes();
    int getRotateLeftTimes();
    int getRotateRightTimes();
    int getReColorTimes();
};

#endif // RBTREE_H

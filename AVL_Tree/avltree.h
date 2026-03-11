#ifndef AVLTREE_H
#define AVLTREE_H

#include "binarysearchtree.h"

class AvlTree : public BinarySearchTree {
public:
  // --- Constructors and Destructor ---
  AvlTree() : BinarySearchTree() {}
  AvlTree(const AvlTree &other) : BinarySearchTree(other) {}
  AvlTree(AvlTree &&other) noexcept : BinarySearchTree(std::move(other)) {}
  ~AvlTree() override = default;

  // --- Operators ---
  AvlTree &operator=(const AvlTree &other);
  AvlTree &operator=(AvlTree &&other) noexcept;

  BinaryTree *clone() const override { return new AvlTree(*this); }

  // --- Subtree Copying ---
  AvlTree *copySubtree(const TreeNode *node) const;

  // --- Overridden Methods ---
  void add(const int value) override;
  bool remove(const int value) override;

protected:
  // --- AVL Specific Recursive Operations ---
  TreeNode *insertAVL(TreeNode *node, const int value);
  TreeNode *removeAVL(TreeNode *node, const int value, bool &wasRemoved);

  // --- Balancing Algorithms ---
  TreeNode *balance(TreeNode *node);
  int getBalanceFactor(TreeNode *node) const;
  TreeNode *rotateRight(TreeNode *y);
  TreeNode *rotateLeft(TreeNode *x);
};

#endif // AVLTREE_H
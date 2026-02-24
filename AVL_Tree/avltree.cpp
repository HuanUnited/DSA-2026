#include "avltree.h"

// ─── Operators ───────────────────────────────────────────────────────────────

AvlTree &AvlTree::operator=(const AvlTree &other) {
  if (this != &other) {
    BinarySearchTree::operator=(other);
  }
  return *this;
}

AvlTree &AvlTree::operator=(AvlTree &&other) noexcept {
  if (this != &other) {
    BinarySearchTree::operator=(std::move(other));
  }
  return *this;
}

// ─── Subtree Copying ─────────────────────────────────────────────────────────

AvlTree *AvlTree::copySubtree(const TreeNode *node) const {
  AvlTree *newTree = new AvlTree();
  if (node) {
    newTree->_root = newTree->_copyNode(node);
  }
  return newTree;
}

// ─── Rotations & Balancing ───────────────────────────────────────────────────

int AvlTree::getBalanceFactor(TreeNode *node) const {
  if (!node)
    return 0;
  return static_cast<int>(_getHeight(node->left())) -
         static_cast<int>(_getHeight(node->right()));
}

BinaryTree::TreeNode *AvlTree::rotateRight(TreeNode *y) {
  TreeNode *x = y->left();
  TreeNode *T2 = x->right();

  x->setRight(y);
  y->setLeft(T2);

  return x;
}

BinaryTree::TreeNode *AvlTree::rotateLeft(TreeNode *x) {
  TreeNode *y = x->right();
  TreeNode *T2 = y->left();

  y->setLeft(x);
  x->setRight(T2);

  return y;
}

BinaryTree::TreeNode *AvlTree::balance(TreeNode *node) {
  if (!node)
    return nullptr;

  int balanceFactor = getBalanceFactor(node);

  // Left Heavy (LL or LR)
  if (balanceFactor > 1) {
    if (getBalanceFactor(node->left()) < 0) {
      node->setLeft(rotateLeft(node->left()));
    }
    return rotateRight(node);
  }

  // Right Heavy (RR or RL)
  if (balanceFactor < -1) {
    if (getBalanceFactor(node->right()) > 0) {
      node->setRight(rotateRight(node->right()));
    }
    return rotateLeft(node);
  }

  return node;
}

// ─── Overridden Modifiers ────────────────────────────────────────────────────

void AvlTree::add(const int value) { _root = insertAVL(_root, value); }

BinaryTree::TreeNode *AvlTree::insertAVL(TreeNode *node, const int value) {
  if (!node)
    return new TreeNode(value);

  if (value < node->value()) {
    node->setLeft(insertAVL(node->left(), value));
  } else if (value > node->value()) {
    node->setRight(insertAVL(node->right(), value));
  } else {
    return node;
  }

  return balance(node);
}

bool AvlTree::remove(const int value) {
  bool wasRemoved = false;
  _root = removeAVL(_root, value, wasRemoved);
  return wasRemoved;
}

BinaryTree::TreeNode *AvlTree::removeAVL(TreeNode *node, const int value,
                                         bool &wasRemoved) {
  if (!node)
    return nullptr;

  if (value < node->value()) {
    node->setLeft(removeAVL(node->left(), value, wasRemoved));
  } else if (value > node->value()) {
    node->setRight(removeAVL(node->right(), value, wasRemoved));
  } else {
    wasRemoved = true;

    if (!node->left()) {
      TreeNode *rightChild = node->right();
      delete node;
      node = rightChild;
    } else if (!node->right()) {
      TreeNode *leftChild = node->left();
      delete node;
      node = leftChild;
    } else {
      int minRightValue = _minSubtree(node->right());
      node->setValue(minRightValue);
      bool dummy = false;
      node->setRight(removeAVL(node->right(), minRightValue, dummy));
    }
  }

  return balance(node);
}
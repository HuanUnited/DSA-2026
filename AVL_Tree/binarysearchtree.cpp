#ifndef BINARY_SEARCH_TREE_CPP
#define BINARY_SEARCH_TREE_CPP

#ifndef BINARY_SEARCH_TREE_H
#include "binarysearchtree.h"
#endif // !BINARY_SEARCH_TREE_H

#include <climits>

void BinarySearchTree::add(const int value) {
  if (!_root) {
    _root = new TreeNode(value);
    return;
  }

  TreeNode *current = _root;
  while (true) {
    if (value < current->value()) {
      if (!current->left()) {
        current->setLeft(new TreeNode(value));
        break;
      }
      current = current->left();
    } else if (value > current->value()) {
      if (!current->right()) {
        current->setRight(new TreeNode(value));
        break;
      }
      current = current->right();
    } else {
      // Value already exists; duplicate handling depends on requirements.
      // Typically, BSTs ignore duplicates or increment a counter.
      break;
    }
  }
}

bool BinarySearchTree::remove(const int value) {
  TreeNode *current = _root;
  TreeNode *parent = nullptr;

  // Find the node
  while (current && current->value() != value) {
    parent = current;
    if (value < current->value()) {
      current = current->left();
    } else {
      current = current->right();
    }
  }

  if (!current)
    return false; // Not found
  // Case 1: Node has two children
  if (current->left() && current->right()) {
    TreeNode *successor = current->right();
    TreeNode *successorParent = current;

    while (successor->left()) {
      successorParent = successor;
      successor = successor->left();
    }

    current->setValue(successor->value());
    current = successor;
    parent = successorParent;
  }

  // Case 2 & 3: Node has one or zero children
  TreeNode *child = current->left() ? current->left() : current->right();

  if (!parent) {
    _root = child;
  } else if (parent->left() == current) {
    parent->setLeft(child);
  } else {
    parent->setRight(child);
  }

  delete current;
  return true;
}

BinaryTree::TreeNode *BinarySearchTree::find(const int value) {
  TreeNode *current = _root;
  while (current) {
    if (value == current->value())
      return current;
    if (value < current->value())
      current = current->left();
    else
      current = current->right();
  }
  return nullptr;
}

int BinarySearchTree::level(const int value) const {
  int currentLevel = 0;
  TreeNode *current = _root;
  while (current) {
    if (value == current->value())
      return currentLevel;
    currentLevel++;
    if (value < current->value())
      current = current->left();
    else
      current = current->right();
  }
  return -1;
}

BinaryTree::TreeNode *BinarySearchTree::_findMin(TreeNode *node) const {
  if (!node)
    return nullptr;
  while (node->left()) {
    node = node->left();
  }
  return node;
}

int BinarySearchTree::min() const {
  TreeNode *minNode = _findMin(_root);
  return minNode ? minNode->value() : INT_MAX;
}

int BinarySearchTree::max() const {
  TreeNode *current = _root;
  if (!current)
    return INT_MIN;
  while (current->right()) {
    current = current->right();
  }
  return current->value();
}

std::vector<int> BinarySearchTree::getVector() const {
  std::vector<int> result;
  // Using the newly implemented LNR iterator
  for (auto it = beginLNR(); it != endLNR(); ++it) {
    result.push_back(*it);
  }
  return result;
}
#endif //! BINARY_SEARCH_TREE_CPP

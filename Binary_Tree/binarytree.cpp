#ifndef BINARY_TREE_CPP
#define BINARY_TREE_CPP

#ifndef BINARY_TREE_H
#include "binarytree.h"
#endif // !BINARY_TREE_H

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <vector>

// --- BinaryTree Method Definitions ---
// --- Helpers ---
BinaryTree::TreeNode *BinaryTree::_copyNode(const TreeNode *sourceNode) {

  if (!sourceNode)
    return nullptr;

  TreeNode *newNode = new TreeNode(sourceNode->value());
  newNode->setLeft(_copyNode(sourceNode->left()));
  newNode->setRight(_copyNode(sourceNode->right()));

  return newNode;
}

void BinaryTree::_deleteSubtree(TreeNode *node) {
  if (!node)
    return;
  _deleteSubtree(node->left());
  _deleteSubtree(node->right());
  delete node;
  node = nullptr;
}

size_t BinaryTree::_getHeight(const TreeNode *node) const {
  if (!node)
    return 0;
  return 1 + std::max(_getHeight(node->left()), _getHeight(node->right()));
}

bool BinaryTree::_balancedSubtree(const TreeNode *node) const {
  if (!node)
    return true;
  size_t heightleft = _getHeight(node->left());
  size_t heightright = _getHeight(node->right());

  size_t diff = (heightleft > heightright) ? (heightleft - heightright)
                                           : (heightright - heightleft);

  if (diff > 1)
    return false;

  return _balancedSubtree(node->left()) && _balancedSubtree(node->right());
}

int BinaryTree::_minSubtree(const TreeNode *node) const {
  if (!node)
    return INT_MAX;
  int res = node->value();
  int leftRes = _minSubtree(node->left());
  int rightRes = _minSubtree(node->right());
  return std::min({res, leftRes, rightRes});
}

int BinaryTree::_maxSubtree(const TreeNode *node) const {
  if (!node)
    return INT_MIN;
  int res = node->value();
  int leftRes = _maxSubtree(node->left());
  int rightRes = _maxSubtree(node->right());
  return std::max({res, leftRes, rightRes});
}

BinaryTree::TreeNode *BinaryTree::_findSubtree(const int value,
                                               TreeNode *node) {
  if (!node)
    return nullptr;
  if (node->value() == value)
    return node;
  TreeNode *lnode{nullptr}, *rnode{nullptr};
  if (node->left())
    lnode = _findSubtree(value, node->left());
  if (lnode != nullptr)
    return lnode;
  if (node->right())
    rnode = _findSubtree(value, node->right());
  if (rnode != nullptr)
    return rnode;
  else
    return nullptr;
}

bool BinaryTree::_isLeaf(const TreeNode *node) const {
  if (!node)
    return false;
  if (!(node->left()) && !(node->right()))
    return true;
  else
    return false;
}

BinaryTree::TreeNode *BinaryTree::_findLeaf(TreeNode *node) {
  if (!node)
    return nullptr; // guard: never recurse into nullptr
  if (_isLeaf(node))
    return node;
  // Try left first, then right; whichever finds a leaf first wins
  TreeNode *found = _findLeaf(node->left());
  if (found)
    return found;
  return _findLeaf(node->right());
}

BinaryTree::TreeNode *BinaryTree::_findParent(TreeNode *root,
                                              TreeNode *target) {
  if (!root || root == target)
    return nullptr;
  if (root->left() == target || root->right() == target)
    return root;
  TreeNode *leftSearch = _findParent(root->left(), target);
  if (leftSearch)
    return leftSearch;
  return _findParent(root->right(), target);
}

int BinaryTree::_findLevel(const TreeNode *node, const int value,
                           const int currentLevel) const {
  if (!node)
    return -1;
  if (node->value() == value)
    return currentLevel;

  int leftLevel = _findLevel(node->left(), value, currentLevel + 1);
  if (leftLevel != -1)
    return leftLevel;

  return _findLevel(node->right(), value, currentLevel + 1);
}

size_t BinaryTree::_calculateSize(const TreeNode *node) const {
  if (!node)
    return 0;
  return (1 + _calculateSize(node->left()) + _calculateSize(node->right()));
}

void BinaryTree::_collectValues(const TreeNode *node,
                                std::vector<int> &values) const {
  if (!node)
    return;
  values.push_back(node->value());
  _collectValues(node->right(), values);
  _collectValues(node->left(), values);
}

void BinaryTree::_printHorizontal(const TreeNode *node, int level) const {
  if (!node)
    return;

  _printHorizontal(node->right(), level + 1);
  std::cout << std::string(level * 4, ' ') << " -> " << node->value() << '\n';
  _printHorizontal(node->left(), level + 1);
}

void BinaryTree::_printLevel(TreeNode *root) const {
  if (!root)
    return;
  std::queue<TreeNode *> q;
  q.push(root);
  int levelCount = 0;
  while (!q.empty()) {
    size_t levelSize = q.size();
    std::cout << "L" << levelCount << ": ";
    for (size_t i = 0; i < levelSize; i++) {
      TreeNode *node = q.front();
      q.pop();
      std::cout << node->value() << " ";
      if (node->left())
        q.push(node->left());
      if (node->right())
        q.push(node->right());
    }
    std::cout << '\n';
    levelCount++;
  }
}

bool BinaryTree::_compareNodes(const TreeNode *node1,
                               const TreeNode *node2) const {
  if (!node1 && !node2)
    return true;
  if ((!node1 && node2) || (node1 && !node2) ||
      node1->value() != node2->value())
    return false;
  return _compareNodes(node1->left(), node2->left()) &&
         _compareNodes(node1->right(), node2->right());
}

// --- Modifiers ---

void BinaryTree::clear() {
  _deleteSubtree(_root);
  _root = nullptr;
}

void BinaryTree::add(const int value) {
  if (!_root) {
    _root = new TreeNode(value);
    return;
  }

  TreeNode *current = _root;
  while (true) {
    bool goLeft = std::rand() & 1;
    if (goLeft) {
      if (!current->left()) {
        current->setLeft(new TreeNode(value));
        break;
      }
      current = current->left();
    } else {
      if (!current->right()) {
        current->setRight(new TreeNode(value));
        break;
      }
      current = current->right();
    }
  }
}

bool BinaryTree::remove(const int value) {
  TreeNode *target = find(value);
  if (!target)
    return false;

  TreeNode *leaf = _findLeaf(_root);
  if (leaf != target) {
    int temp = leaf->value();
    leaf->setValue(target->value());
    target->setValue(temp);
  }
  TreeNode *parent = _findParent(_root, leaf);
  if (parent) {
    if (parent->left() == leaf)
      parent->setLeft(nullptr);
    else
      parent->setRight(nullptr);

  } else {
    _root = nullptr;
  }
  delete leaf;
  return true;
}

void BinaryTree::removeSubtree(int const value) {
  if (!_root)
    return;

  if (_root->value() == value) {
    clear();
    return;
  }

  TreeNode *target = find(value);
  if (!target)
    return;

  TreeNode *parent = _findParent(_root, target);
  if (parent) {
    if (parent->left() == target)
      parent->setLeft(nullptr);
    else
      parent->setRight(nullptr);
  }

  _deleteSubtree(target);
}

// --- Operators ---
bool BinaryTree::operator==(BinaryTree const &other) const {
  return _compareNodes(_root, other._root);
}

BinaryTree &BinaryTree::operator=(const BinaryTree &other) noexcept {
  BinaryTree tempTree(other);
  std::swap(tempTree._root, _root);
  return *this;
}

BinaryTree &BinaryTree::operator=(BinaryTree &&other) noexcept {
  std::swap(other._root, _root);
  return *this;
}

// --- Output and Visualization ---
std::vector<int> BinaryTree::getVector() const {
  std::vector<int> res;
  _collectValues(_root, res);
  std::sort(res.begin(), res.end());
  return res;
}

void BinaryTree::print(bool byLevel) {
  if (byLevel) {
    _printLevel(_root);
  } else {
    _printHorizontal(_root, 0);
  }
}

#endif // !BINARY_TREE_CPP

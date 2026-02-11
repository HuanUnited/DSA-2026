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
TreeNode *BinaryTree::_copyNode(const TreeNode *sourceNode) {
  if (!sourceNode)
    return nullptr;

  TreeNode *newNode = new TreeNode(sourceNode->_value);
  newNode->_left = _copyNode(sourceNode->_left);
  newNode->_right = _copyNode(sourceNode->_right);

  return newNode;
}

void BinaryTree::_deleteSubtree(TreeNode *node) {
  if (!node)
    return;
  _deleteSubtree(node->_left);
  _deleteSubtree(node->_right);
  delete node;
  node = nullptr;
}

size_t BinaryTree::_getHeight(const TreeNode *node) const {
  if (!node)
    return 0;
  return 1 + std::max(_getHeight(node->_left), _getHeight(node->_right));
}

bool BinaryTree::_balancedSubtree(const TreeNode *node) const {
  if (!node)
    return true;
  size_t height_left = _getHeight(node->_left);
  size_t height_right = _getHeight(node->_right);

  size_t diff = (height_left > height_right) ? (height_left - height_right)
                                             : (height_right - height_left);

  if (diff > 1)
    return false;

  return _balancedSubtree(node->_left) && _balancedSubtree(node->_right);
}

int BinaryTree::_minSubtree(const TreeNode *node) const {
  if (!node)
    return INT_MAX;
  int res = node->_value;
  int leftRes = _minSubtree(node->_left);
  int rightRes = _minSubtree(node->_right);
  return std::min({res, leftRes, rightRes});
}

int BinaryTree::_maxSubtree(const TreeNode *node) const {
  if (!node)
    return INT_MIN;
  int res = node->_value;
  int leftRes = _maxSubtree(node->_left);
  int rightRes = _maxSubtree(node->_right);
  return std::max({res, leftRes, rightRes});
}

TreeNode *BinaryTree::_findSubtree(const int value, TreeNode *node) {
  if (!node)
    return nullptr;
  if (node->_value == value)
    return node;
  TreeNode *lnode{nullptr}, *rnode{nullptr};
  if (node->_left)
    lnode = _findSubtree(value, node->_left);
  if (lnode != nullptr)
    return lnode;
  if (node->_right)
    rnode = _findSubtree(value, node->_right);
  if (rnode != nullptr)
    return rnode;
  else
    return nullptr;
}

bool BinaryTree::_isLeaf(const TreeNode *node) const {
  if (!node)
    return false;
  if (!(node->_left) && !(node->_right))
    return true;
  else
    return false;
}

TreeNode *BinaryTree::_findLeaf(TreeNode *node) {
  if (_isLeaf(node))
    return node;
  else {
    TreeNode *lbranch = _findLeaf(node->_left);
    if (_isLeaf(lbranch))
      return lbranch;
    TreeNode *rbranch = _findLeaf(node->_right);
    if (_isLeaf(rbranch))
      return rbranch;
    else
      return nullptr;
  }
}

TreeNode *BinaryTree::_findParent(TreeNode *root, TreeNode *target) {
  if (!root || root == target)
    return nullptr;
  if (root->_left == target || root->_right == target)
    return root;
  TreeNode *leftSearch = _findParent(root->_left, target);
  if (leftSearch)
    return leftSearch;
  return _findParent(root->_right, target);
}

int BinaryTree::_findLevel(const TreeNode *node, const int value,
                           const int currentLevel) const {
  if (!node)
    return -1;
  if (node->_value == value)
    return currentLevel;

  int leftLevel = _findLevel(node->_left, value, currentLevel + 1);
  if (leftLevel != -1)
    return leftLevel;

  return _findLevel(node->_right, value, currentLevel + 1);
}

size_t BinaryTree::_calculateSize(const TreeNode *node) {
  if (!node)
    return 0;
  return (1 + _calculateSize(node->_left) + _calculateSize(node->_right));
}

void BinaryTree::_collectValues(const TreeNode *node,
                                std::vector<int> &values) const {
  if (!node)
    return;
  values.push_back(node->_value);
  _collectValues(node->_right, values);
  _collectValues(node->_left, values);
}

void BinaryTree::_printHorizontal(const TreeNode *node, int level) const {
  if (!node)
    return;

  _printHorizontal(node->_right, level + 1);
  std::cout << std::string(level * 4, ' ') << " -> " << node->_value << '\n';
  _printHorizontal(node->_left, level + 1);
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
      std::cout << node->_value << " ";
      if (node->_left)
        q.push(node->_left);
      if (node->_right)
        q.push(node->_right);
    }
    std::cout << '\n';
    levelCount++;
  }
}

bool BinaryTree::_compareNodes(const TreeNode *node1,
                               const TreeNode *node2) const {
  if (!node1 && !node2)
    return true;
  if ((!node1 && node2) || (node1 && !node2) || node1->_value != node2->_value)
    return false;
  return _compareNodes(node1->_left, node2->_left) &&
         _compareNodes(node1->_right, node2->_right);
}

// --- Modifiers ---

void BinaryTree::clear() {
  _deleteSubtree(_root);
  _root = nullptr;
  _size = 0;
}

void BinaryTree::add(const int value) {
  if (!_root) {
    _root = new TreeNode(value);
    _size++;
    return;
  }

  TreeNode *current = _root;
  while (true) {
    bool goLeft = std::rand() & 1;
    if (goLeft) {
      if (!current->_left) {
        current->_left = new TreeNode(value);
        break;
      }
      current = current->_left;
    } else {
      if (!current->_right) {
        current->_right = new TreeNode(value);
        break;
      }
      current = current->_right;
    }
  }
  _size++;
}

bool BinaryTree::remove(const int value) {
  TreeNode *target = find(value);
  if (!target)
    return false;

  TreeNode *leaf = _findLeaf(_root);
  if (leaf != target) {
    std::swap(target->_value, leaf->_value);
  }
  TreeNode *parent = _findParent(_root, leaf);
  if (parent) {
    if (parent->_left == leaf)
      parent->_left = nullptr;
    else
      parent->_right = nullptr;

  } else {
    _root = nullptr;
  }
  delete leaf;
  --_size;
  return true;
}

void BinaryTree::removeSubtree(int const value) {
  if (!_root)
    return;

  if (_root->_value == value) {
    clear();
    return;
  }

  TreeNode *target = find(value);
  if (!target)
    return;

  TreeNode *parent = _findParent(_root, target);
  if (parent) {
    if (parent->_left == target)
      parent->_left = nullptr;
    else
      parent->_right = nullptr;
  }

  _deleteSubtree(target);
  _size = _calculateSize(_root);
}

// --- Operators ---
bool BinaryTree::operator==(BinaryTree const &other) const {
  if (_size != other._size)
    return false;
  return _compareNodes(_root, other._root);
}

BinaryTree &BinaryTree::operator=(const BinaryTree &other) noexcept {
  BinaryTree tempTree(other);
  std::swap(tempTree._root, _root);
  std::swap(tempTree._size, _size);
  return *this;
}

BinaryTree &BinaryTree::operator=(BinaryTree &&other) noexcept {
  std::swap(other._root, _root);
  std::swap(other._size, _size);
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

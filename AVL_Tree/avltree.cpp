#include "avltree.h"
// --- Height / Nodes ----------------------------------------------------------

int8_t AvlTree::_getNodeHeight(TreeNode *node) const {
  // Standard AVL height lookup: nullptr is 0, otherwise return stored height
  return node ? static_cast<AVLNode *>(node)->_height : 0;
}

void AvlTree::_updateHeight(TreeNode *node) const {
  if (node) {
    // Height is 1 + the height of the tallest child
    static_cast<AVLNode *>(node)->_height =
        1 +
        std::max(_getNodeHeight(node->left()), _getNodeHeight(node->right()));
  }
}

int AvlTree::getBalanceFactor(TreeNode *node) const {
  if (!node)
    return 0;
  // Direct subtraction of stored heights
  return _getNodeHeight(node->left()) - _getNodeHeight(node->right());
}

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

BinaryTree::TreeNode *AvlTree::rotateRight(TreeNode *y) {
  TreeNode *x = y->left();
  TreeNode *T2 = x->right();

  x->setRight(y);
  y->setLeft(T2);

  _updateHeight(y);
  _updateHeight(x);

  return x;
}

BinaryTree::TreeNode *AvlTree::rotateLeft(TreeNode *x) {
  TreeNode *y = x->right();
  TreeNode *T2 = y->left();

  y->setLeft(x);
  x->setRight(T2);

  _updateHeight(y);
  _updateHeight(x);

  return y;
}

BinaryTree::TreeNode *AvlTree::balance(TreeNode *node) {
  if (!node)
    return nullptr;

  _updateHeight(node);
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
    return new AVLNode(value);

  if (value < node->value()) {
    node->setLeft(insertAVL(node->left(), value));
  } else if (value > node->value()) {
    node->setRight(insertAVL(node->right(), value));
  } else {
    return node;
  }

  // Update height locally (O(1))
  _updateHeight(node);

  // Short-circuit: Only call the complex balancing logic if the node is
  // actually tilted
  int bf = getBalanceFactor(node);
  if (bf < -1 || bf > 1) {
    return balance(node);
  }

  return node;
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

    // Case 1 & 2: One or Zero children
    if (!node->left() || !node->right()) {
      TreeNode *temp = node->left() ? node->left() : node->right();

      if (!temp) { // No children
        delete node;
        return nullptr; // Return nullptr to the parent's setLeft/setRight
      } else {          // One child
        delete node;
        node = temp;
      }
    } else {
      int minRightValue = _minSubtree(node->right());
      node->setValue(minRightValue);
      bool dummy = false;
      node->setRight(removeAVL(node->right(), minRightValue, dummy));
    }
  }

  _updateHeight(node);

  // Conditional balancing.
  int bf = getBalanceFactor(node);
  if (bf < -1 || bf > 1) {
    return balance(node);
  }

  return node;
}
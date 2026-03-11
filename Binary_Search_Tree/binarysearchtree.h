#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include "binarytree.h"
#include <stack>

class BinarySearchTree : public BinaryTree {
public:
  // --- LNR (In-Order) Iterator ---
  template <bool IsConst> class LNRIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<IsConst, const int *, int *>;
    using reference = std::conditional_t<IsConst, const int &, int &>;
    using node_ptr = std::conditional_t<IsConst, const TreeNode *, TreeNode *>;

    LNRIterator(node_ptr root = nullptr) { _pushLeft(root); }

    reference operator*() const { return _stack.top()->value(); }
    pointer operator->() const { return &(_stack.top()->value()); }

    LNRIterator &operator++() {
      if (_stack.empty())
        return *this;
      node_ptr curr = _stack.top();
      _stack.pop();
      if (curr->right()) {
        _pushLeft(curr->right());
      }
      return *this;
    }

    LNRIterator operator++(int) {
      LNRIterator<IsConst> tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const LNRIterator &other) const {
      if (_stack.empty() && other._stack.empty())
        return true;
      if (_stack.empty() || other._stack.empty())
        return false;
      return _stack.top() == other._stack.top();
    }

    bool operator!=(const LNRIterator &other) const {
      return !(*this == other);
    }

  private:
    std::stack<node_ptr> _stack;

    void _pushLeft(node_ptr node) {
      while (node) {
        _stack.push(node);
        node = node->left();
      }
    }
  };

  using lnr_iterator = LNRIterator<false>;
  using const_lnr_iterator = LNRIterator<true>;

  // --- Constructors and Destructor ---
  BinarySearchTree() : BinaryTree() {}
  BinarySearchTree(const BinarySearchTree &other) : BinaryTree(other) {}
  BinarySearchTree(BinarySearchTree &&other) noexcept
      : BinaryTree(std::move(other)) {}
  ~BinarySearchTree() override = default;

  // --- Operators ---
  BinarySearchTree &operator=(const BinarySearchTree &other) {
    if (this != &other) {
      BinaryTree::operator=(other);
    }
    return *this;
  }

  BinarySearchTree &operator=(BinarySearchTree &&other) noexcept {
    if (this != &other) {
      BinaryTree::operator=(std::move(other));
    }
    return *this;
  }

  // --- LNR Iterators ---
  lnr_iterator beginLNR() { return lnr_iterator(_root); }
  lnr_iterator endLNR() { return lnr_iterator(nullptr); }
  const_lnr_iterator beginLNR() const { return const_lnr_iterator(_root); }
  const_lnr_iterator endLNR() const { return const_lnr_iterator(nullptr); }

  // --- Overridden Methods ---
  void add(const int value) override;
  bool remove(const int value) override;
  TreeNode *find(const int value) override;
  int level(const int value) const override;
  int min() const override;
  int max() const override;
  std::vector<int> getVector() const override;
  BinaryTree *clone() const override { return new BinarySearchTree(*this); }

private:
  TreeNode *_findMin(TreeNode *node) const;
};

#endif // BINARY_SEARCH_TREE_H

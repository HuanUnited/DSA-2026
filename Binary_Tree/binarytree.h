#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <cstddef>
#include <iterator>
#include <queue>
#include <type_traits>
#include <vector>

class BinaryTree {
public:
  class TreeNode {
  private:
    int _value;
    TreeNode *_left;
    TreeNode *_right;

  public:
    // --- Constructors ---

    TreeNode() : _value(0), _left(nullptr), _right(nullptr) {};

    explicit TreeNode(int val, TreeNode *left = nullptr,
                      TreeNode *right = nullptr)
        : _value(val), _left(left), _right(right) {};

    // --- Getters and Setters ---

    int &value() { return _value; }
    const int &value() const { return _value; }

    void setValue(int const value) { _value = value; };

    TreeNode *left() const { return _left; };

    TreeNode *right() const { return _right; };

    void setLeft(TreeNode *left) { _left = left; };

    void setRight(TreeNode *right) { _right = right; };
  };

  template <bool IsConst> class Iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<IsConst, const int *, int *>;
    using reference = std::conditional_t<IsConst, const int &, int &>;
    using node_ptr = std::conditional_t<IsConst, const TreeNode *, TreeNode *>;

    Iterator(TreeNode *root = nullptr) {
      if (root)
        _nodes.push(root);
    }

    reference operator*() const { return _nodes.front()->value(); }
    pointer operator->() const { return &(_nodes.front()->value()); }

    Iterator &operator++() {
      if (!_nodes.empty()) {
        node_ptr curr = _nodes.front();
        _nodes.pop();

        if (curr->left())
          _nodes.push(curr->left());
        if (curr->right())
          _nodes.push(curr->right());
      }

      return *this;
    }

    Iterator operator++(int) {
      Iterator<IsConst> tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const Iterator &other) const {
      if (_nodes.empty() && other._nodes.empty())
        return true;
      if (_nodes.empty() || other._nodes.empty())
        return false;
      return _nodes.front() == other._nodes.front();
    }

    bool operator!=(const Iterator &other) const { return !(*this == other); }

  private:
    std::queue<TreeNode *> _nodes;
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  // --- Constructors and Destructors ---
  BinaryTree() : _root(nullptr) {};

  BinaryTree(BinaryTree const &other) { _root = _copyNode(other._root); }

  BinaryTree(BinaryTree &&other) noexcept {
    _root = nullptr;
    std::swap(_root, other._root);
  }

  ~BinaryTree() { clear(); }

  // --- Capacity and Status ---

  bool empty() const { return (bool)(_root); }

  size_t height() const { return _getHeight(_root); }

  size_t size() const { return _calculateSize(_root); };

  bool balanced() const { return _balancedSubtree(_root); };

  // --- Element Access ---

  const TreeNode *root() const { return _root; }

  TreeNode *root() { return _root; };

  TreeNode *find(const int value) { return _findSubtree(value, _root); }

  int level(const int value) const { return _findLevel(_root, value, 0); }

  int min() const { return _minSubtree(_root); }

  int max() const { return _maxSubtree(_root); }

  // --- Iterator ---
  iterator begin() { return iterator(_root); }

  iterator end() { return iterator(); }

  const_iterator begin() const { return const_iterator(_root); }
  const_iterator end() const { return const_iterator(); }

  const_iterator cbegin() const { return begin(); }

  const_iterator cend() const { return end(); }

  // --- Modifiers ---

  void add(const int value);

  bool remove(const int value);

  void clear();

  void removeSubtree(int const value);

  // --- Output and Visualization ---

  std::vector<int> getVector() const;

  void print(bool byLevel = false);

  // --- Operators ---

  bool operator==(BinaryTree const &other) const;

  BinaryTree &operator=(const BinaryTree &other) noexcept;

  BinaryTree &operator=(BinaryTree &&other) noexcept;

private:
  TreeNode *_root;

  // --- Helpers ---
  TreeNode *_copyNode(const TreeNode *sourceNode);

  void _deleteSubtree(TreeNode *node);

  size_t _getHeight(const TreeNode *node) const;

  bool _balancedSubtree(const TreeNode *node) const;

  int _minSubtree(const TreeNode *node) const;

  int _maxSubtree(const TreeNode *node) const;

  TreeNode *_findSubtree(const int value, TreeNode *node);

  bool _isLeaf(const TreeNode *node) const;

  TreeNode *_findLeaf(TreeNode *node);

  TreeNode *_findParent(TreeNode *root, TreeNode *target);

  int _findLevel(const TreeNode *node, const int value,
                 const int currentLevel) const;

  size_t _calculateSize(const TreeNode *node) const;

  void _collectValues(const TreeNode *node, std::vector<int> &values) const;

  void _printHorizontal(const TreeNode *node, int space) const;

  void _printLevel(TreeNode *root) const;

  bool _compareNodes(const TreeNode *node1, const TreeNode *node2) const;
};

#endif // !BINARY_TREE_H

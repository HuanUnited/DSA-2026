#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <cstddef>
#include <iterator>
#include <queue>
#include <type_traits>
#include <vector>

/**
 * @class TreeNode
 * @brief A single node of the BinaryTree
 *
 * Stores an integer key and pointers to the left and right children.
 * This is implemented as a struct for easier data access as per requirements.
 */
struct TreeNode {
  int _value;
  TreeNode *_left;
  TreeNode *_right;

  // --- Constructors ---

  /** @brief Default constructor. Initializes children to nullptr. */
  TreeNode() : _value(0), _left(nullptr), _right(nullptr) {};

  /** @brief Parameterized constructor. */
  explicit TreeNode(int val, TreeNode *left = nullptr,
                    TreeNode *right = nullptr)
      : _value(val), _left(left), _right(right) {};

  // --- Getters and Setters ---

  /** @return The current integer key of the node. */
  int value() const { return _value; };

  /** @param value The new integer value to the be assigned to the node. */
  void setValue(int const value) { _value = value; };

  /** @return The pointer to the left child node. */
  TreeNode *left() const { return _left; };

  /** @return The pointer to the right child node. */
  TreeNode *right() const { return _right; };

  /** @param left Pointer to the left child node. */
  void setLeft(TreeNode *left) { _left = left; };

  /** @param right Pointer to the right child node. */
  void setRight(TreeNode *right) { _right = right; };
};

/**
 *  @class BinaryTree
 *
 *  @brief This is a non-linear container where nodes are placed via randomized
 * subtree selection rather than sorted order.
 */
class BinaryTree {
public:
  /**@class Iterator
   *
   * @brief This an implemented iterator class used to navigate the BinaryTree
   * safely.
   */
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

    reference operator*() const { return _nodes.front()->_value; }
    pointer operator->() const { return &(_nodes.front()->_value); }

    Iterator &operator++() {
      if (!_nodes.empty()) {
        node_ptr curr = _nodes.front();
        _nodes.pop();

        if (curr->_left)
          _nodes.push(curr->_left);
        if (curr->_right)
          _nodes.push(curr->_right);
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
  /** @brief Default constructor. Initializes an empty BinaryTree */
  BinaryTree() : _root(nullptr), _size(0) {};

  /** @brief Copy Constructor. */
  BinaryTree(BinaryTree const &other) {
    _root = _copyNode(other._root);
    _size = other._size;
  }

  /** @brief Move Constructor. */
  BinaryTree(BinaryTree &&other) noexcept {
    _root = nullptr;
    _size = 0;
    std::swap(_root, other._root);
    std::swap(_size, other._size);
  }

  /** @brief Destructor. */
  ~BinaryTree() { clear(); }

  // --- Capacity and Status ---

  /** @return The root node of the BinaryTree. */
  bool empty() const { return (bool)(_root); }

  /** @return The height of the BinaryTree. */
  size_t height() const { return _getHeight(_root); }

  /** @return The amount of nodes in the BinaryTree. */
  size_t size() const { return _size; };

  /** @return If the BinaryTree is balanced or not.*/
  bool balanced() const { return _balancedSubtree(_root); };

  // --- Element Access ---

  /** @return The root node of the BinaryTree. */
  const TreeNode *root() const { return _root; }

  /**@return The modifiable root of the BinaryTree. */
  TreeNode *root() { return _root; };

  /** @param value The value of the required Node.
   * @return The first found Node with the given value.
   */
  TreeNode *find(const int value) { return _findSubtree(value, _root); }

  /**@param value The value of the needed Node
   * @return The distance of the first found node with the given Value to the
   * root.*/

  int level(const int value) const { return _findLevel(_root, value, 0); }

  /** @return The minimum value in the BinaryTree. */
  int min() const { return _minSubtree(_root); }

  /** @return The maximum value in the BinaryTree. */
  int max() const { return _maxSubtree(_root); }

  // --- Iterator ---
  iterator begin() { return iterator(_root); }

  iterator end() { return iterator(); }

  const_iterator begin() const { return const_iterator(_root); }
  const_iterator end() const { return const_iterator(); }

  const_iterator cbegin() const { return begin(); }

  const_iterator cend() const { return end(); }

  // --- Modifiers ---

  /** @param value The value of the added Node.
   *  @brief Adds a new node by randomly choosing a Subtree.
   */
  void add(const int value);

  /** @param value The value of the first removed Node.
   * @brief Removes the first found Node with the given value.
   */
  bool remove(const int value);

  /**@brief Clears the BinaryTree of all nodes.*/
  void clear();

  /**@param value The value of the needed node.
   * @brief Deletes the Subtree corresponding to the first found node of the
   * given value*/
  void removeSubtree(int const value);

  // --- Output and Visualization ---

  /** @return An STL Vector with all the elements of the BinaryTree in ascending
   * order.*/
  std::vector<int> getVector() const;

  // TODO: QT Visualization

  void print(bool byLevel = false);

  // --- Operators ---

  /** @brief Comparison operator */
  bool operator==(BinaryTree const &other) const;

  /** @brief Copy Assignment Operator*/
  BinaryTree &operator=(const BinaryTree &other) noexcept;

  /** @brief Move Assignment Operator*/
  BinaryTree &operator=(BinaryTree &&other) noexcept;

private:
  TreeNode *_root;
  size_t _size;

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

  size_t _calculateSize(const TreeNode *node);

  void _collectValues(const TreeNode *node, std::vector<int> &values) const;

  void _printHorizontal(const TreeNode *node, int space) const;

  void _printLevel(TreeNode *root) const;

  bool _compareNodes(const TreeNode *node1, const TreeNode *node2) const;
};

#endif // !BINARY_TREE_H

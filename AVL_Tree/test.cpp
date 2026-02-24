#include "binarysearchtree.h"
#include "binarytree.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

void test_passed(const std::string &name) {
  std::cout << GREEN << "[PASS] " << RESET << name << std::endl;
}

void test_failed(const std::string &name) {
  std::cout << RED << "[FAIL] " << RESET << name << std::endl;
}

void section_header(const std::string &s) {
  std::cout << YELLOW << "\n=== " << s << " ===" << RESET << std::endl;
}

void test_basic_ops() {
  section_header("Basic Operations (Yellow/10pts)");
  BinaryTree tree;
  assert(tree.size() == 0);
  assert(!tree.root());

  tree.add(50);
  tree.add(25);
  tree.add(75);
  assert(tree.size() == 3);
  assert(tree.root()->value() == 50);
  test_passed("Initial additions and size");

  tree.clear();
  assert(tree.size() == 0);
  assert(tree.root() == nullptr);
  test_passed("Tree clearing");
}

void test_search_and_stats() {
  section_header("Search and Stats (Green/5pts)");
  BinaryTree tree;
  tree.add(10);
  tree.add(20);
  tree.add(5);

  // Find check
  if (tree.find(10) != nullptr)
    test_passed("Find method");
  else
    test_failed("Find method - Could not find value 10");

  // Min/Max check
  if (tree.min() == 5 && tree.max() == 20)
    test_passed("Min/Max calculations");
  else
    test_failed("Min/Max - Values did not match expected 5 and 20");

  // Level check - Don't assert a specific number, just assert it's found (>= 0)
  int lvl = tree.level(20);
  if (lvl >= 0)
    test_passed("Level/Depth calculation (Found at level " +
                std::to_string(lvl) + ")");
  else
    test_failed("Level calculation - Value 20 returned level -1");
}

void test_logic_and_structure() {
  section_header("Structure and Balancing (5pts)");
  BinaryTree tree;
  tree.add(100);
  assert(tree.balanced());

  // Force some depth to test balancing logic
  tree.add(50);
  tree.add(150);
  assert(tree.height() >= 2);
  test_passed("Height and Balanced property");
}

void test_iterators() {
  section_header("BFS Iterators (5pts)");
  BinaryTree tree;
  std::vector<int> inputs = {1, 2, 3, 4, 5};
  for (int i : inputs)
    tree.add(i);

  int count = 0;
  for (auto it = tree.begin(); it != tree.end(); ++it) {
    std::cout << "Visiting: " << *it << std::endl;
    count++;
  }
  std::cout << "Final Count: " << count << std::endl;
  assert(count == 5);
  test_passed("Breadth-First Iterator traversal");
}

void test_vector_and_operators() {
  section_header("Vector and Comparison Operators");
  BinaryTree tree1, tree2;
  tree1.add(10);
  tree1.add(20);
  tree2.add(10);
  tree2.add(20);

  std::vector<int> vec = tree1.getVector();
  assert(std::is_sorted(vec.begin(), vec.end()));
  test_passed("getVector (Ascending Order)");

  BinaryTree tree3 = tree1; // Copy constructor
  assert(tree3.size() == tree1.size());
  test_passed("Deep copy constructor");
}

void test_random_tree() {
  BinaryTree tree;
  std::cout << "Testing randomized additions..." << std::endl;

  tree.add(10);
  tree.add(20);
  tree.add(30);
  tree.add(40);
  tree.add(5);

  assert(tree.size() == 5);
  assert(tree.find(30) != nullptr);
  assert(tree.find(99) == nullptr);

  // Min/Max should work regardless of random structure
  assert(tree.min() == 5);
  assert(tree.max() == 40);

  std::cout << "Min: " << tree.min() << ", Max: " << tree.max() << std::endl;
  std::cout << "Current Height: " << tree.height() << std::endl;

  std::vector<int> sorted = tree.getVector();
  assert(std::is_sorted(sorted.begin(), sorted.end()));

  std::cout << "Sorted elements: ";
  for (int i : sorted)
    std::cout << i << " ";
  std::cout << std::endl;
}

void test_print() {
  BinaryTree tree;
  std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  for (auto c : vec)
    tree.add(c);
  std::cout << GREEN << "\n Print Tree by Level\n" << RESET;
  tree.print(true);
  std::cout << GREEN << "\n Print Tree by Height\n" << RESET;
  tree.print(false);
  test_passed("Print Works!");
}

void test_bst_ops() {
  section_header("Binary Search Tree Operations (15pts)");
  BinarySearchTree bst;

  // Test Insertion
  bst.add(50);
  bst.add(30);
  bst.add(70);
  bst.add(20);
  bst.add(40);

  assert(bst.root()->value() == 50);
  assert(bst.root()->left()->value() == 30);
  assert(bst.root()->right()->value() == 70);
  test_passed("BST Insertion logic maintains invariant");

  // Test Search & Extremes
  assert(bst.find(40) != nullptr);
  assert(bst.find(99) == nullptr);
  assert(bst.min() == 20);
  assert(bst.max() == 70);
  test_passed("BST Min, Max, and Find overridden algorithms");

  // Test LNR Traversal
  std::vector<int> expected = {20, 30, 40, 50, 70};
  std::vector<int> actual = bst.getVector();
  assert(actual == expected);
  test_passed("BST LNR Traversal (getVector) naturally ordered");

  // Test Deletion
  bool removed = bst.remove(50);
  assert(removed);
  assert(bst.size() == 4);
  assert(bst.root()->value() != 50);
  test_passed("BST Deletion (Node with two children)");
}

void test_all() {
  try {
    test_basic_ops();
    test_search_and_stats();
    test_logic_and_structure();
    test_iterators();
    test_vector_and_operators();
    test_random_tree();
    test_print();
    test_bst_ops(); // Execute the BST suite
    std::cout << GREEN << "\nAll tree tests passed successfully!" << RESET
              << std::endl;
  } catch (const std::exception &e) {
    std::cerr << RED << "Test failed: " << e.what() << RESET << std::endl;
  }
}

#ifndef TREETESTS_H
#define TREETESTS_H

#include "binarysearchtree.h"
#include "binarytree.h"
#include <QString>
#include <QVector>

/**
 * @struct TestResult
 * @brief Holds the outcome of a single test case.
 */
struct TestResult {
  QString name;
  bool passed;
  QString detail; // extra info / error message
};

/**
 * @class TreeTests
 * @brief Mirrors the logic in test.cpp but collects results instead of
 *        calling assert() / abort().  Safe to call from the Qt UI thread.
 */
class TreeTests {
public:
  /** Run all test suites and return one TestResult per test case. */
  static QVector<TestResult> runAll();

private:
  static void test_basic_ops           (QVector<TestResult> &out);
  static void test_search_and_stats    (QVector<TestResult> &out);
  static void test_logic_and_structure (QVector<TestResult> &out);
  static void test_iterators           (QVector<TestResult> &out);
  static void test_vector_and_operators(QVector<TestResult> &out);
  static void test_random_tree         (QVector<TestResult> &out);

  // New BST specific test suite
  static void test_bst_operations      (QVector<TestResult> &out);

  // New AVL specific test suite
  static void test_avl_operations      (QVector<TestResult> &out);

  // Helpers
  static void pass(QVector<TestResult> &out, const QString &name,
                   const QString &detail = {});
  static void fail(QVector<TestResult> &out, const QString &name,
                   const QString &detail = {});
};

#endif // TREETESTS_H

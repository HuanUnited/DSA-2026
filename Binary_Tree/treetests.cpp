#include "treetests.h"
#include <algorithm>
#include <vector>

// ── Helpers ──────────────────────────────────────────────────────────────────

void TreeTests::pass(QVector<TestResult> &out,
                     const QString &name, const QString &detail)
{
    out.push_back({name, true, detail});
}

void TreeTests::fail(QVector<TestResult> &out,
                     const QString &name, const QString &detail)
{
    out.push_back({name, false, detail});
}

// ── Entry point ──────────────────────────────────────────────────────────────

QVector<TestResult> TreeTests::runAll() {
    QVector<TestResult> results;
    test_basic_ops           (results);
    test_search_and_stats    (results);
    test_logic_and_structure (results);
    test_iterators           (results);
    test_vector_and_operators(results);
    test_random_tree         (results);
    return results;
}

// ── Suite: Basic Operations ───────────────────────────────────────────────────

void TreeTests::test_basic_ops(QVector<TestResult> &out) {
    BinaryTree tree;

    // Empty tree
    if (tree.size() == 0 && !tree.root())
        pass(out, "Empty tree has size 0 and no root");
    else
        fail(out, "Empty tree has size 0 and no root",
             QString("size=%1 root=%2").arg(tree.size()).arg((quintptr)tree.root()));

    tree.add(50); tree.add(25); tree.add(75);

    if (tree.size() == 3)
        pass(out, "Size is 3 after three additions");
    else
        fail(out, "Size is 3 after three additions",
             QString("Got size=%1").arg(tree.size()));

    if (tree.root() && tree.root()->value() == 50)
        pass(out, "Root value is 50");
    else
        fail(out, "Root value is 50",
             QString("Got %1").arg(tree.root() ? tree.root()->value() : -1));

    tree.clear();

    if (tree.size() == 0 && tree.root() == nullptr)
        pass(out, "Tree clear() resets size to 0 and root to nullptr");
    else
        fail(out, "Tree clear() resets size to 0 and root to nullptr",
             QString("size=%1").arg(tree.size()));
}

// ── Suite: Search & Stats ─────────────────────────────────────────────────────

void TreeTests::test_search_and_stats(QVector<TestResult> &out) {
    BinaryTree tree;
    tree.add(10); tree.add(20); tree.add(5);

    if (tree.find(10) != nullptr)
        pass(out, "find(10) returns non-null");
    else
        fail(out, "find(10) returns non-null", "Returned nullptr");

    if (tree.find(999) == nullptr)
        pass(out, "find(999) returns nullptr for missing value");
    else
        fail(out, "find(999) returns nullptr for missing value", "Returned non-null");

    if (tree.min() == 5)
        pass(out, "min() == 5");
    else
        fail(out, "min() == 5", QString("Got %1").arg(tree.min()));

    if (tree.max() == 20)
        pass(out, "max() == 20");
    else
        fail(out, "max() == 20", QString("Got %1").arg(tree.max()));

    int lvl = tree.level(20);
    if (lvl >= 0)
        pass(out, "level(20) >= 0 (found in tree)",
             QString("level=%1").arg(lvl));
    else
        fail(out, "level(20) >= 0 (found in tree)", "Returned -1");

    if (tree.level(999) == -1)
        pass(out, "level(999) == -1 for missing node");
    else
        fail(out, "level(999) == -1 for missing node",
             QString("Got %1").arg(tree.level(999)));
}

// ── Suite: Structure & Balance ────────────────────────────────────────────────

void TreeTests::test_logic_and_structure(QVector<TestResult> &out) {
    BinaryTree tree;
    tree.add(100);

    if (tree.balanced())
        pass(out, "Single-node tree is balanced");
    else
        fail(out, "Single-node tree is balanced", "balanced() returned false");

    tree.add(50); tree.add(150);

    if (tree.height() >= 2)
        pass(out, "Height >= 2 after three nodes",
             QString("height=%1").arg(tree.height()));
    else
        fail(out, "Height >= 2 after three nodes",
             QString("Got height=%1").arg(tree.height()));
}

// ── Suite: BFS Iterator ───────────────────────────────────────────────────────

void TreeTests::test_iterators(QVector<TestResult> &out) {
    BinaryTree tree;
    for (int i : {1, 2, 3, 4, 5}) tree.add(i);

    int count = 0;
    for (auto it = tree.begin(); it != tree.end(); ++it)
        ++count;

    if (count == 5)
        pass(out, "BFS iterator visits all 5 nodes",
             QString("visited=%1").arg(count));
    else
        fail(out, "BFS iterator visits all 5 nodes",
             QString("visited=%1").arg(count));

    // const_iterator
    int ccount = 0;
    const BinaryTree &ct = tree;
    for (auto it = ct.begin(); it != ct.end(); ++it) ++ccount;

    if (ccount == 5)
        pass(out, "const_iterator visits all 5 nodes");
    else
        fail(out, "const_iterator visits all 5 nodes",
             QString("visited=%1").arg(ccount));
}

// ── Suite: Vector & Operators ─────────────────────────────────────────────────

void TreeTests::test_vector_and_operators(QVector<TestResult> &out) {
    BinaryTree tree1;
    tree1.add(10); tree1.add(20); tree1.add(5); tree1.add(1); tree1.add(15);

    auto vec = tree1.getVector();

    if (std::is_sorted(vec.begin(), vec.end()))
        pass(out, "getVector() returns values in ascending order");
    else
        fail(out, "getVector() returns values in ascending order",
             "Vector is not sorted");

    if (static_cast<int>(vec.size()) == 5)
        pass(out, "getVector() contains all 5 elements");
    else
        fail(out, "getVector() contains all 5 elements",
             QString("Got size=%1").arg(vec.size()));

    // Copy constructor
    BinaryTree tree2 = tree1;

    if (tree2.size() == tree1.size())
        pass(out, "Copy constructor: sizes match",
             QString("size=%1").arg(tree2.size()));
    else
        fail(out, "Copy constructor: sizes match",
             QString("tree1=%1 tree2=%2").arg(tree1.size()).arg(tree2.size()));

    // Modifying tree2 should not affect tree1
    tree2.add(999);
    if (tree1.size() != tree2.size())
        pass(out, "Copy is a deep copy (modifying copy does not affect original)");
    else
        fail(out, "Copy is a deep copy (modifying copy does not affect original)",
             "Both trees have the same size after modifying copy");

    // Equality operator
    BinaryTree tree3 = tree1;
    if (tree1 == tree3)
        pass(out, "operator== returns true for equal trees");
    else
        fail(out, "operator== returns true for equal trees");

    if (!(tree1 == tree2))
        pass(out, "operator== returns false for unequal trees");
    else
        fail(out, "operator== returns false for unequal trees");
}

// ── Suite: Random Tree ────────────────────────────────────────────────────────

void TreeTests::test_random_tree(QVector<TestResult> &out) {
    BinaryTree tree;
    for (int v : {10, 20, 30, 40, 5}) tree.add(v);

    if (tree.size() == 5)
        pass(out, "Random tree: size == 5 after 5 additions");
    else
        fail(out, "Random tree: size == 5 after 5 additions",
             QString("Got %1").arg(tree.size()));

    if (tree.find(30) != nullptr)
        pass(out, "Random tree: find(30) succeeds");
    else
        fail(out, "Random tree: find(30) succeeds", "Returned nullptr");

    if (tree.find(99) == nullptr)
        pass(out, "Random tree: find(99) returns nullptr");
    else
        fail(out, "Random tree: find(99) returns nullptr");

    if (tree.min() == 5)
        pass(out, "Random tree: min() == 5");
    else
        fail(out, "Random tree: min() == 5", QString("Got %1").arg(tree.min()));

    if (tree.max() == 40)
        pass(out, "Random tree: max() == 40");
    else
        fail(out, "Random tree: max() == 40", QString("Got %1").arg(tree.max()));

    auto sorted = tree.getVector();
    if (std::is_sorted(sorted.begin(), sorted.end()))
        pass(out, "Random tree: getVector() is sorted");
    else
        fail(out, "Random tree: getVector() is sorted", "Not sorted");
}

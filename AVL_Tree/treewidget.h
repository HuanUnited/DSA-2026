#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <climits>
#include "binarytree.h"

/**
 * @class TreeWidget
 * @brief Custom QPainter widget that renders a BinaryTree visually.
 *
 * Supports three independent highlight modes:
 *   Find  -> red node
 *   Min   -> green node
 *   Max   -> orange node
 */
class TreeWidget : public QWidget {
    Q_OBJECT

public:
    enum class HighlightMode { None, Find, Min, Max };

    explicit TreeWidget(QWidget *parent = nullptr);

    void setTree(const BinaryTree *tree);
    void setHighlight(int value, HighlightMode mode = HighlightMode::Find);
    void clearHighlight();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const BinaryTree *_tree          = nullptr;
    int               _highlightVal  = INT_MIN;
    HighlightMode     _highlightMode = HighlightMode::None;

    static constexpr int NODE_RADIUS  = 22;
    static constexpr int LEVEL_HEIGHT = 80;

    void _drawSubtree(QPainter &painter,
                      const BinaryTree::TreeNode *node,
                      int x, int y, int hSpacing) const;

    QColor _nodeColor(int nodeValue) const;
};

#endif // TREEWIDGET_H

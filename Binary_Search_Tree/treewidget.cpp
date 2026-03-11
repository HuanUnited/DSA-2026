#include "treewidget.h"
#include <QPen>
#include <QFont>

TreeWidget::TreeWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setStyleSheet("background-color: #f5f5f5;"
                  "border: 1px solid #cccccc;"
                  "border-radius: 6px;");
}

void TreeWidget::setTree(const BinaryTree *tree) {
    _tree = tree;
    update();
}

void TreeWidget::setHighlight(int value, HighlightMode mode) {
    _highlightVal  = value;
    _highlightMode = mode;
    update();
}

void TreeWidget::clearHighlight() {
    _highlightVal  = INT_MIN;
    _highlightMode = HighlightMode::None;
    update();
}

// ── Color logic ──────────────────────────────────────────────────────────────

QColor TreeWidget::_nodeColor(int nodeValue) const {
    if (_highlightMode != HighlightMode::None && nodeValue == _highlightVal) {
        switch (_highlightMode) {
            case HighlightMode::Find: return QColor("#e74c3c"); // red
            case HighlightMode::Min:  return QColor("#27ae60"); // green
            case HighlightMode::Max:  return QColor("#e67e22"); // orange
            default: break;
        }
    }
    return QColor("#3a7bd5"); // default blue
}

// ── Paint ────────────────────────────────────────────────────────────────────

void TreeWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!_tree || !_tree->root()) {
        painter.setPen(QColor("#888888"));
        painter.setFont(QFont("Segoe UI", 11));
        painter.drawText(rect(), Qt::AlignCenter,
                         "Tree is empty.\nUse 'Add' to insert nodes.");
        return;
    }

    // Legend for active highlight
    if (_highlightMode != HighlightMode::None) {
        QString legendText;
        QColor  legendColor;
        if (_highlightMode == HighlightMode::Find) {
            legendText = QString("Find: %1").arg(_highlightVal);
            legendColor = QColor("#e74c3c");
        } else if (_highlightMode == HighlightMode::Min) {
            legendText = QString("Min: %1").arg(_highlightVal);
            legendColor = QColor("#27ae60");
        } else {
            legendText = QString("Max: %1").arg(_highlightVal);
            legendColor = QColor("#e67e22");
        }
        painter.setBrush(legendColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(10, 10, 14, 14);
        painter.setPen(QColor("#333333"));
        painter.setFont(QFont("Segoe UI", 9));
        painter.drawText(30, 22, legendText);
    }

    const int height   = static_cast<int>(_tree->height());
    const int hSpacing = std::max(20, (width() / 2) / (height + 1));

    _drawSubtree(painter, _tree->root(),
                 width() / 2, NODE_RADIUS + 20,
                 hSpacing);
}

// ── Recursive draw ────────────────────────────────────────────────────────────

void TreeWidget::_drawSubtree(QPainter &painter,
                               const BinaryTree::TreeNode *node,
                               int x, int y, int hSpacing) const
{
    if (!node) return;

    const int nextHSpacing = std::max(1, hSpacing / 2);

    // Edges first (behind nodes)
    if (node->left()) {
        int cx = x - hSpacing, cy = y + LEVEL_HEIGHT;
        painter.setPen(QPen(QColor("#aaaaaa"), 2));
        painter.drawLine(x, y, cx, cy);
        _drawSubtree(painter, node->left(), cx, cy, nextHSpacing);
    }
    if (node->right()) {
        int cx = x + hSpacing, cy = y + LEVEL_HEIGHT;
        painter.setPen(QPen(QColor("#aaaaaa"), 2));
        painter.drawLine(x, y, cx, cy);
        _drawSubtree(painter, node->right(), cx, cy, nextHSpacing);
    }

    // Node circle
    QColor fill   = _nodeColor(node->value());
    QColor border = fill.darker(130);

    painter.setPen(QPen(border, 2));
    painter.setBrush(fill);
    painter.drawEllipse(QPoint(x, y), NODE_RADIUS, NODE_RADIUS);

    // Value label
    painter.setPen(Qt::white);
    painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
    painter.drawText(QRect(x - NODE_RADIUS, y - NODE_RADIUS,
                           NODE_RADIUS * 2, NODE_RADIUS * 2),
                     Qt::AlignCenter,
                     QString::number(node->value()));
}

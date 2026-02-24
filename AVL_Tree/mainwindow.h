#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "binarysearchtree.h"
#include "binarytree.h"
#include "treewidget.h"
#include <QMainWindow>
#include <memory>

class QLineEdit;
class QTextEdit;
class QPushButton;
class QLabel;
class QComboBox; // Forward declaration for the selector

/**
 * @class MainWindow
 * @brief Qt front-end for the BinaryTree class.
 *
 * Layout
 * ──────
 *  ┌──────────────────────────────────────────────────────────┐
 *  │  Control Panel (left)  │  Tree 1 (top-right)             │
 *  │                        ├─────────────────────────────────│
 *  │                        │  Tree 2 / Compare (bottom-right) │
 *  ├──────────────────────────────────────────────────────────┤
 *  │  Output / Test log (full width, bottom)                  │
 *  └──────────────────────────────────────────────────────────┘
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override = default;

private slots:
  // --- Mutators ---
  void on_addButton_clicked();
  void on_removeButton_clicked();
  void on_removeSubtreeButton_clicked();
  void on_clearButton_clicked();

  // --- Queries ---
  void on_findButton_clicked();
  void on_levelButton_clicked();
  void on_heightButton_clicked();
  void on_sizeButton_clicked();
  void on_minButton_clicked();
  void on_maxButton_clicked();
  void on_balancedButton_clicked();
  void on_vectorButton_clicked();

  // --- Tree 2 / Compare ---
  void on_duplicateButton_clicked();
  void on_compareButton_clicked();
  void on_clearTree2Button_clicked();

  // --- Tests ---
  void on_runTestsButton_clicked();

  // --- Utility ---
  void on_clearOutputButton_clicked();
  void on_treeTypeChanged(int index);

private:
  // Widgets
  TreeWidget *_treeWidget1;
  TreeWidget *_treeWidget2;
  QLineEdit *_valueInput;
  QTextEdit *_outputDisplay;
  QLabel *_tree2StatusLabel;
  QComboBox  *_treeTypeSelector;

  // Data
  std::unique_ptr<BinaryTree> _tree;
  std::unique_ptr<BinaryTree> _tree2;

  // Helpers
  void _buildUi();
  void _updateTrees();
  void _log(const QString &msg);
  void _logHtml(const QString &html);
  bool _getInputValue(int &out);

  QPushButton *_makeButton(const QString &label, const QString &color,
                           const char *slot);
};

#endif // MAINWINDOW_H

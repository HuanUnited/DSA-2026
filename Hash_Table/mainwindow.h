#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "hashtable_qt.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void onInsertClicked();
  void onRemoveClicked();
  void onExistClicked();
  void onResizeClicked();
  void onHashFunctionChanged(int index);
  void updateTable();

private:
  HashTable<std::string> *hashTable;
  QTableWidget *tableWidget;

  QLineEdit *keyInput;
  QLineEdit *valueInput;
  QLineEdit *removeKeyInput;
  QLineEdit *existKeyInput;
  QSpinBox *resizeSpinBox;
  QComboBox *hashFunctionCombo;

  QPushButton *insertButton;
  QPushButton *removeButton;
  QPushButton *existButton;
  QPushButton *resizeButton;

  QLabel *statusLabel;

  void setupUI();
};

#endif // MAINWINDOW_H

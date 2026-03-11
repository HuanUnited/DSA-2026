#include "mainwindow.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  hashTable = new HashTable<std::string>(10);
  setupUI();
  updateTable();
}

MainWindow::~MainWindow() { delete hashTable; }

void MainWindow::setupUI() {
  setWindowTitle("Хеш-таблица - Лабораторная работа");
  setMinimumSize(800, 600);

  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

  // Заголовок
  QLabel *titleLabel = new QLabel("Визуализация хеш-таблицы", this);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(16);
  titleFont.setBold(true);
  titleLabel->setFont(titleFont);
  titleLabel->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(titleLabel);

  // Секция выбора хеш-функции
  QGroupBox *hashFunctionGroup = new QGroupBox("Хеш-функция", this);
  QHBoxLayout *hashFunctionLayout = new QHBoxLayout(hashFunctionGroup);

  QLabel *hashFunctionLabel = new QLabel("Выберите хеш-функцию:", this);
  hashFunctionCombo = new QComboBox(this);
  hashFunctionCombo->addItem("Квадратичная: hi(K) = (h0 + c×i + d×i²) mod N");
  hashFunctionCombo->addItem("Рекурсивная: hi(K) = [hi-1×a×N] mod N");
  hashFunctionCombo->addItem("Двойное хеширование: hi(K) = ((K mod N) + i×(1+K mod (N-2))) mod N");

  hashFunctionLayout->addWidget(hashFunctionLabel);
  hashFunctionLayout->addWidget(hashFunctionCombo, 1);

  mainLayout->addWidget(hashFunctionGroup);

  connect(hashFunctionCombo, SIGNAL(currentIndexChanged(int)), this,
          SLOT(onHashFunctionChanged(int)));

  // Секция управления
  QGroupBox *controlGroup = new QGroupBox("Операции с таблицей", this);
  QGridLayout *controlLayout = new QGridLayout(controlGroup);

  // Вставка
  QLabel *insertLabel = new QLabel("Добавить элемент:", this);
  controlLayout->addWidget(insertLabel, 0, 0);

  QLabel *keyLabel = new QLabel("Ключ:", this);
  keyInput = new QLineEdit(this);
  keyInput->setPlaceholderText("Введите ключ (число)");
  controlLayout->addWidget(keyLabel, 0, 1);
  controlLayout->addWidget(keyInput, 0, 2);

  QLabel *valueLabel = new QLabel("Значение:", this);
  valueInput = new QLineEdit(this);
  valueInput->setPlaceholderText("Введите значение");
  controlLayout->addWidget(valueLabel, 0, 3);
  controlLayout->addWidget(valueInput, 0, 4);

  insertButton = new QPushButton("Добавить", this);
  controlLayout->addWidget(insertButton, 0, 5);
  connect(insertButton, &QPushButton::clicked, this,
          &MainWindow::onInsertClicked);

  // Удаление
  QLabel *removeLabel = new QLabel("Удалить элемент:", this);
  controlLayout->addWidget(removeLabel, 1, 0);

  QLabel *removeKeyLabel = new QLabel("Ключ:", this);
  removeKeyInput = new QLineEdit(this);
  removeKeyInput->setPlaceholderText("Введите ключ для удаления");
  controlLayout->addWidget(removeKeyLabel, 1, 1);
  controlLayout->addWidget(removeKeyInput, 1, 2);

  removeButton = new QPushButton("Удалить", this);
  controlLayout->addWidget(removeButton, 1, 3);
  connect(removeButton, &QPushButton::clicked, this,
          &MainWindow::onRemoveClicked);

  // Проверка существования
  QLabel *existLabel = new QLabel("Проверить наличие:", this);
  controlLayout->addWidget(existLabel, 2, 0);

  QLabel *existKeyLabel = new QLabel("Ключ:", this);
  existKeyInput = new QLineEdit(this);
  existKeyInput->setPlaceholderText("Введите ключ для проверки");
  controlLayout->addWidget(existKeyLabel, 2, 1);
  controlLayout->addWidget(existKeyInput, 2, 2);

  existButton = new QPushButton("Проверить", this);
  controlLayout->addWidget(existButton, 2, 3);
  connect(existButton, &QPushButton::clicked, this,
          &MainWindow::onExistClicked);

  // Изменение размера
  QLabel *resizeLabel = new QLabel("Изменить размер:", this);
  controlLayout->addWidget(resizeLabel, 3, 0);

  resizeSpinBox = new QSpinBox(this);
  resizeSpinBox->setMinimum(5);
  resizeSpinBox->setMaximum(100);
  resizeSpinBox->setValue(10);
  controlLayout->addWidget(resizeSpinBox, 3, 1);

  resizeButton = new QPushButton("Изменить размер", this);
  controlLayout->addWidget(resizeButton, 3, 2);
  connect(resizeButton, &QPushButton::clicked, this,
          &MainWindow::onResizeClicked);

  mainLayout->addWidget(controlGroup);

  // Таблица для отображения хеш-таблицы
  QGroupBox *tableGroup = new QGroupBox("Содержимое хеш-таблицы", this);
  QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);

  tableWidget = new QTableWidget(this);
  tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  tableWidget->horizontalHeader()->setStretchLastSection(true);

  tableLayout->addWidget(tableWidget);
  mainLayout->addWidget(tableGroup, 1);

  // Строка состояния
  statusLabel = new QLabel("Готово к работе", this);
  statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: gray; }");
  mainLayout->addWidget(statusLabel);
}

void MainWindow::onInsertClicked() {
  bool ok;
  unsigned int key = keyInput->text().toUInt(&ok);

  if (!ok) {
    QMessageBox::warning(this, "Ошибка", "Ключ должен быть целым числом!");
    return;
  }

  std::string value = valueInput->text().toStdString();

  if (value.empty()) {
    QMessageBox::warning(this, "Ошибка", "Значение не может быть пустым!");
    return;
  }

  hashTable->insert(key, value);
  updateTable();

  keyInput->clear();
  valueInput->clear();

  statusLabel->setText(QString("Добавлен элемент: ключ %1, значение '%2'")
                           .arg(key)
                           .arg(QString::fromStdString(value)));
}

void MainWindow::onRemoveClicked() {
  bool ok;
  unsigned int key = removeKeyInput->text().toUInt(&ok);

  if (!ok) {
    QMessageBox::warning(this, "Ошибка", "Ключ должен быть целым числом!");
    return;
  }

  if (hashTable->exist(key)) {
    hashTable->remove(key);
    updateTable();
    statusLabel->setText(QString("Удалён элемент с ключом %1").arg(key));
  } else {
    statusLabel->setText(QString("Элемент с ключом %1 не найден").arg(key));
  }

  removeKeyInput->clear();
}

void MainWindow::onExistClicked() {
  bool ok;
  unsigned int key = existKeyInput->text().toUInt(&ok);

  if (!ok) {
    QMessageBox::warning(this, "Ошибка", "Ключ должен быть целым числом!");
    return;
  }

  if (hashTable->exist(key)) {
    statusLabel->setText(QString("Элемент с ключом %1 найден в таблице").arg(key));
  } else {
    statusLabel->setText(QString("Элемент с ключом %1 не найден").arg(key));
  }

  existKeyInput->clear();
}

void MainWindow::onResizeClicked() {
  int newSize = resizeSpinBox->value();
  hashTable->resize(newSize);
  updateTable();

  statusLabel->setText(QString("Размер таблицы изменён на %1").arg(newSize));
}

void MainWindow::onHashFunctionChanged(int index) {
  HashType newType;

  switch (index) {
  case 0:
    newType = HashType::Quadratic;
    break;
  case 1:
    newType = HashType::Recursive;
    break;
  case 2:
    newType = HashType::Double;
    break;
  default:
    newType = HashType::Quadratic;
  }

  hashTable->setHashFunction(newType);
  updateTable();

  QString hashName;
  switch (index) {
  case 0:
    hashName = "Квадратичная";
    break;
  case 1:
    hashName = "Рекурсивная";
    break;
  case 2:
    hashName = "Двойное хеширование";
    break;
  }

  statusLabel->setText(QString("Хеш-функция изменена на: %1").arg(hashName));
}

void MainWindow::updateTable() {
  hashTable->printToQt(tableWidget);
}

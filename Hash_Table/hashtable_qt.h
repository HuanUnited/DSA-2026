#ifndef HASHTABLE_QT
#define HASHTABLE_QT

#include "qheaderview.h"
#include <QTableWidget>
#include <cmath>
#include <iostream>
#include <list>
#include <sstream>
#include <utility>
#include <vector>

constexpr int kGroupNumber = 13;
constexpr unsigned int kC = kGroupNumber % 5;
constexpr unsigned int kD = kGroupNumber % 7;
const double kA = -1 * (1.0 - std::sqrt(5.0)) / 2.0;

enum class HashType { Quadratic, Recursive, Double };

template <typename T> class HashTable {
private:
  size_t _tableSize = 10;
  std::vector<std::list<std::pair<unsigned int, T>>> _table;

  HashType _currentType = HashType::Quadratic;

  unsigned int _getHash(unsigned int const key, unsigned int iteration) const;

  // Вспомогательная функция для преобразования значения в строку
  std::string valueToString(const T &value) const;

public:
  HashTable(size_t size = 10) : _tableSize(size), _table(size) {}
  HashTable(const HashTable &other);
  ~HashTable();

  void insert(unsigned int const key, T const element);
  void remove(unsigned int const key);
  bool exist(unsigned int const key) const;
  void swap(HashTable &other);
  void print() const;
  void printToQt(QTableWidget *tableWidget) const; // Вывод на форму Qt

  void resize(size_t const size);
  void rehash();

  void setHashFunction(const HashType newtype);

  HashTable &operator=(const HashTable &other);
  T &operator[](const unsigned int key);
};

// Вспомогательная функция для преобразования значения в строку
template <typename T>
std::string HashTable<T>::valueToString(const T &value) const {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

// Вычисление хеш-значения в зависимости от типа хеш-функции
template <typename T>
unsigned int HashTable<T>::_getHash(unsigned int const key,
                                     unsigned int iteration) const {
  unsigned int h0 = key % _tableSize;

  switch (_currentType) {
  case HashType::Quadratic:
    return (h0 + kC * iteration + kD * iteration * iteration) % _tableSize;

  case HashType::Recursive: {
    unsigned int hash = h0;
    for (unsigned int i = 0; i < iteration; ++i) {
      hash = static_cast<unsigned int>(std::floor(hash * kA * _tableSize)) %
             _tableSize;
    }
    return hash;
  }

  case HashType::Double:
    return (h0 + iteration * (1 + key % (_tableSize - 2))) % _tableSize;

  default:
    return h0;
  }
}

// Конструктор копирования
template <typename T>
HashTable<T>::HashTable(const HashTable &other)
    : _tableSize(other._tableSize), _table(other._table),
      _currentType(other._currentType) {}

// Деструктор
template <typename T> HashTable<T>::~HashTable() {}

// Вставка элемента
template <typename T>
void HashTable<T>::insert(unsigned int const key, T const element) {
  for (unsigned int i = 0; i < _tableSize; ++i) {
    unsigned int currentIndex = _getHash(key, i);

    for (auto &pair : _table[currentIndex]) {
      if (pair.first == key) {
        pair.second = element;
        return;
      }
    }

    _table[currentIndex].push_back({key, element});
    return;
  }

  std::cerr << "Hash table is full, cannot insert key " << key << std::endl;
}

// Удаление элемента по ключу
template <typename T> void HashTable<T>::remove(unsigned int const key) {
  for (unsigned int i = 0; i < _tableSize; ++i) {
    unsigned int currentIndex = _getHash(key, i);

    for (auto it = _table[currentIndex].begin();
         it != _table[currentIndex].end(); ++it) {
      if (it->first == key) {
        _table[currentIndex].erase(it);
        return;
      }
    }

    if (_table[currentIndex].empty()) {
      return;
    }
  }
}

// Проверка наличия элемента
template <typename T>
bool HashTable<T>::exist(unsigned int const key) const {
  for (unsigned int i = 0; i < _tableSize; ++i) {
    unsigned int currentIndex = _getHash(key, i);

    for (const auto &pair : _table[currentIndex]) {
      if (pair.first == key) {
        return true;
      }
    }

    if (_table[currentIndex].empty()) {
      return false;
    }
  }

  return false;
}

// Обмен содержимым с другой таблицей
template <typename T> void HashTable<T>::swap(HashTable &other) {
  std::swap(_tableSize, other._tableSize);
  std::swap(_table, other._table);
  std::swap(_currentType, other._currentType);
}

// Вывод содержимого таблицы в консоль
template <typename T> void HashTable<T>::print() const {
  for (size_t i = 0; i < _tableSize; ++i) {
    std::cout << "Hash " << i << ": ";

    if (_table[i].empty()) {
      std::cout << "(empty)";
    } else {
      bool first = true;
      for (const auto &pair : _table[i]) {
        if (!first) {
          std::cout << ", ";
        }
        std::cout << "[" << pair.first << ": " << pair.second << "]";
        first = false;
      }
    }

    std::cout << std::endl;
  }
}

// Вывод содержимого таблицы на форму Qt
template <typename T>
void HashTable<T>::printToQt(QTableWidget *tableWidget) const {
  if (!tableWidget)
    return;

  // Очищаем таблицу
  tableWidget->clear();

  // Настраиваем количество столбцов и их заголовки
  tableWidget->setColumnCount(2);
  tableWidget->setHorizontalHeaderLabels(QStringList() << "Хеш" << "Ключ: Значение");

  // Подсчитываем общее количество строк
  int totalRows = 0;
  for (size_t i = 0; i < _tableSize; ++i) {
    if (_table[i].empty()) {
      totalRows++; // Одна строка для пустой ячейки
    } else {
      totalRows += _table[i].size(); // По строке на каждый элемент
    }
  }

  tableWidget->setRowCount(totalRows);

  // Заполняем таблицу
  int currentRow = 0;
  for (size_t i = 0; i < _tableSize; ++i) {
    if (_table[i].empty()) {
      // Пустая ячейка
      tableWidget->setItem(currentRow, 0,
                           new QTableWidgetItem(QString::number(i)));
      tableWidget->setItem(currentRow, 1,
                           new QTableWidgetItem("(пусто)"));
      currentRow++;
    } else {
      // Ячейка с элементами
      bool firstInHash = true;
      for (const auto &pair : _table[i]) {
        // Хеш отображаем только для первого элемента
        if (firstInHash) {
          tableWidget->setItem(currentRow, 0,
                               new QTableWidgetItem(QString::number(i)));
          firstInHash = false;
        } else {
          tableWidget->setItem(currentRow, 0, new QTableWidgetItem(""));
        }

        // Отображаем ключ и значение
        QString keyValue = QString("[%1: %2]")
                               .arg(pair.first)
                               .arg(QString::fromStdString(valueToString(pair.second)));
        tableWidget->setItem(currentRow, 1, new QTableWidgetItem(keyValue));

        currentRow++;
      }
    }
  }

  // Настраиваем размеры столбцов
  tableWidget->resizeColumnsToContents();
  tableWidget->horizontalHeader()->setStretchLastSection(true);
}

// Изменение размера хеш-таблицы
template <typename T> void HashTable<T>::resize(size_t const size) {
  if (size == 0) {
    std::cerr << "Cannot resize to 0" << std::endl;
    return;
  }

  std::vector<std::list<std::pair<unsigned int, T>>> oldTable = _table;
  size_t oldSize = _tableSize;

  _tableSize = size;
  _table.clear();
  _table.resize(_tableSize);

  for (size_t i = 0; i < oldSize; ++i) {
    for (const auto &pair : oldTable[i]) {
      insert(pair.first, pair.second);
    }
  }
}

// Пересчёт позиций элементов с текущей хеш-функцией
template <typename T> void HashTable<T>::rehash() {
  std::vector<std::pair<unsigned int, T>> allElements;

  for (size_t i = 0; i < _tableSize; ++i) {
    for (const auto &pair : _table[i]) {
      allElements.push_back(pair);
    }
  }

  _table.clear();
  _table.resize(_tableSize);

  for (const auto &pair : allElements) {
    insert(pair.first, pair.second);
  }
}

// Замена хеш-функции
template <typename T>
void HashTable<T>::setHashFunction(const HashType newtype) {
  _currentType = newtype;
  rehash();
}

// Оператор присваивания
template <typename T>
HashTable<T> &HashTable<T>::operator=(const HashTable &other) {
  if (this != &other) {
    _tableSize = other._tableSize;
    _table = other._table;
    _currentType = other._currentType;
  }
  return *this;
}

// Получение ссылки на значение по ключу
template <typename T> T &HashTable<T>::operator[](const unsigned int key) {
  for (unsigned int i = 0; i < _tableSize; ++i) {
    unsigned int currentIndex = _getHash(key, i);

    for (auto &pair : _table[currentIndex]) {
      if (pair.first == key) {
        return pair.second;
      }
    }

    if (_table[currentIndex].empty()) {
      _table[currentIndex].push_back({key, T()});
      return _table[currentIndex].back().second;
    }
  }

  unsigned int lastIndex = _getHash(key, _tableSize - 1);
  _table[lastIndex].push_back({key, T()});
  return _table[lastIndex].back().second;
}

#endif // HASHTABLE_QT

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

#ifndef I_HASH_FUNCTION
#define I_HASH_FUNCTION

// Интерфейс внешней хэш-функции h1(K) для метода цепочек
class IHashFunction {
public:
    virtual ~IHashFunction() = default;

    [[nodiscard]] virtual unsigned int CalcHash(unsigned int key, unsigned int tableSize) const = 0;

    [[nodiscard]] virtual IHashFunction *clone() const = 0;
};

// Хэш-функция на основе квадратичного зондирования при i = 1
class QuadraticHash : public IHashFunction {
public:
    [[nodiscard]] unsigned int CalcHash(unsigned int key, unsigned int tableSize) const override {
        unsigned int h0 = key % tableSize;
        return (h0 + kC + kD) % tableSize;
    }

    [[nodiscard]] IHashFunction *clone() const override {
        return new QuadraticHash(*this);
    }
};

// Хэш-функция на основе рекурсивного зондирования при i = 1
class RecursiveHash : public IHashFunction {
public:
    [[nodiscard]] unsigned int CalcHash(unsigned int key, unsigned int tableSize) const override {
        unsigned int h0 = key % tableSize;
        return static_cast<unsigned int>(std::floor(h0 * kA * tableSize)) % tableSize;
    }

    [[nodiscard]] IHashFunction *clone() const override {
        return new RecursiveHash(*this);
    }
};

// Хэш-функция на основе двойного хэширования при i = 1
class DoubleHash : public IHashFunction {
public:
    [[nodiscard]] unsigned int CalcHash(unsigned int key, unsigned int tableSize) const override {
        unsigned int h0 = key % tableSize;
        unsigned int h2 = 1 + key % (tableSize - 2);
        return (h0 + h2) % tableSize;
    }

    [[nodiscard]] IHashFunction *clone() const override {
        return new DoubleHash(*this);
    }
};

#endif // I_HASH_FUNCTION


template<typename T>
class HashTable {
private:
    size_t _tableSize = 10;
    std::vector<std::list<std::pair<unsigned int, T> > > _table;

    IHashFunction *_hashFunction = nullptr;

    [[nodiscard]] unsigned int _getHash(unsigned int key) const;

    // Вспомогательная функция для преобразования значения в строку
    static std::string valueToString(const T &value);

    void _rehash();

public:
    HashTable(size_t size = 10) : _tableSize(size), _table(size), _hashFunction(new QuadraticHash()) {
    }

    HashTable(const HashTable &other);

    ~HashTable();

    void insert(unsigned int key, T element);

    void remove(unsigned int key);

    [[nodiscard]] bool exist(unsigned int key) const;

    void swap(HashTable &other) noexcept;

    void print() const;

    void printToQt(QTableWidget *tableWidget) const;

    void resize(size_t size);


    void setHashFunction(const IHashFunction *newHash);

    HashTable &operator=(const HashTable &other);

    T &operator[](unsigned int key);
};

// Вспомогательная функция для преобразования значения в строку
template<typename T>
std::string HashTable<T>::valueToString(const T &value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Вычисление хеш-значения в зависимости от типа хеш-функции
template<typename T>
unsigned int HashTable<T>::_getHash(unsigned int const key) const {
    if (_hashFunction) {
        return _hashFunction->CalcHash(key, _tableSize);
    }
    return key % _tableSize;
}

// Конструктор копирования
template<typename T>
HashTable<T>::HashTable(const HashTable &other)
    : _tableSize(other._tableSize), _table(other._table) {
    if (other._hashFunction) {
        _hashFunction = other._hashFunction->clone();
    } else {
        _hashFunction = new QuadraticHash();
    }
}

// Деструктор
template<typename T>
HashTable<T>::~HashTable() {
    delete _hashFunction;
}

// Вставка элемента
template<typename T>
void HashTable<T>::insert(unsigned int const key, T const element) {
    unsigned int currentIndex = _getHash(key);

    for (auto &pair: _table[currentIndex]) {
        if (pair.first == key) {
            pair.second = element;
            return;
        }
    }

    _table[currentIndex].push_back({key, element});
}

// Удаление элемента по ключу
template<typename T>
void HashTable<T>::remove(unsigned int const key) {
    unsigned int currentIndex = _getHash(key);

    for (auto it = _table[currentIndex].begin();
         it != _table[currentIndex].end(); ++it) {
        if (it->first == key) {
            _table[currentIndex].erase(it);
            return;
        }
    }
}

// Проверка наличия элемента
template<typename T>
bool HashTable<T>::exist(unsigned int const key) const {
    unsigned int currentIndex = _getHash(key);

    for (const auto &pair: _table[currentIndex]) {
        if (pair.first == key) {
            return true;
        }
    }

    return false;
}

// Обмен содержимым с другой таблицей
template<typename T>
void HashTable<T>::swap(HashTable &other) noexcept {
    std::swap(_tableSize, other._tableSize);
    std::swap(_table, other._table);
    std::swap(_hashFunction, other._hashFunction);
}

// Вывод содержимого таблицы в консоль
template<typename T>
void HashTable<T>::print() const {
    for (size_t i = 0; i < _tableSize; ++i) {
        std::cout << "Hash " << i << ": ";

        if (_table[i].empty()) {
            std::cout << "(empty)";
        } else {
            bool first = true;
            for (const auto &pair: _table[i]) {
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
template<typename T>
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
            totalRows++;
        } else {
            totalRows += _table[i].size();
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
            for (const auto &pair: _table[i]) {
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
template<typename T>
void HashTable<T>::resize(size_t const size) {
    if (size == 0) {
        std::cerr << "Cannot resize to 0" << std::endl;
        return;
    }

    std::vector<std::list<std::pair<unsigned int, T> > > oldTable = _table;
    size_t oldSize = _tableSize;

    _tableSize = size;
    _table.clear();
    _table.resize(_tableSize);

    for (size_t i = 0; i < oldSize; ++i) {
        for (const auto &pair: oldTable[i]) {
            insert(pair.first, pair.second);
        }
    }
}

// Пересчёт позиций элементов с текущей хеш-функцией
template<typename T>
void HashTable<T>::_rehash() {
    std::vector<std::pair<unsigned int, T> > allElements;

    for (size_t i = 0; i < _tableSize; ++i) {
        for (const auto &pair: _table[i]) {
            allElements.push_back(pair);
        }
    }

    _table.clear();
    _table.resize(_tableSize);

    for (const auto &pair: allElements) {
        insert(pair.first, pair.second);
    }
}

// Замена хеш-функции
template<typename T>
void HashTable<T>::setHashFunction(const IHashFunction *newHash) {
    if (newHash) {
        delete _hashFunction;
        _hashFunction = newHash->clone();
        _rehash();
    }
}

// Оператор присваивания
template<typename T>
HashTable<T> &HashTable<T>::operator=(const HashTable &other) {
    if (this != &other) {
        _tableSize = other._tableSize;
        _table = other._table;
        delete _hashFunction;
        if (other._hashFunction) {
            _hashFunction = other._hashFunction->clone();
        } else {
            _hashFunction = new QuadraticHash();
        }
    }
    return *this;
}

// Получение ссылки на значение по ключу
template<typename T>
T &HashTable<T>::operator[](const unsigned int key) {
    unsigned int currentIndex = _getHash(key);

    for (auto &pair: _table[currentIndex]) {
        if (pair.first == key) {
            return pair.second;
        }
    }

    _table[currentIndex].push_back({key, T()});
    return _table[currentIndex].back().second;
}

#endif // HASHTABLE_QT

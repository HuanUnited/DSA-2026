#ifndef HASHTABLE
#define HASHTABLE

#include <cmath>
#include <iostream>
#include <list>
#include <ostream>
#include <utility>
#include <vector>

constexpr int kGroupNumber = 13;
constexpr unsigned int kC = kGroupNumber / 5;
constexpr unsigned int kD = kGroupNumber / 7;
const double kA = -1 * (1.0 - std::sqrt(5.0)) / 2.0;

enum class HashType { Quadratic, Recursive, Double };

template <typename T> class HashTable {
private:
  size_t _tableSize = 10;
  std::vector<std::list<std::pair<unsigned int, T>>> _table;

  HashType _currentType = HashType::Quadratic;

  unsigned int _getHash(unsigned int const key, unsigned int iteration) const;

public:
  HashTable(size_t size = 10) : _tableSize(size), _table(size) {}
  HashTable(const HashTable &other);
  ~HashTable();

  void insert(unsigned int const key, T const element);
  void remove(unsigned int const key);
  bool exist(unsigned int const key) const;
  void swap(HashTable &other);
  void print() const;

  void resize(size_t const size);
  void rehash();

  void setHashFunction(const HashType newtype);

  HashTable &operator=(const HashTable &other);
  T &operator[](const unsigned int key);
};

// Вычисление хеш-значения в зависимости от типа хеш-функции
template <typename T>
unsigned int HashTable<T>::_getHash(unsigned int const key,
                                    unsigned int iteration) const {
  unsigned int h0 = key % _tableSize;

  switch (_currentType) {
  case HashType::Quadratic:
    // hi(K) = (h0(K) + c × i + d × i²) mod N
    return (h0 + kC * iteration + kD * iteration * iteration) % _tableSize;

  case HashType::Recursive: {
    // hi(K) = [hi-1(K) × a × N] mod N
    // Для i=0: h0(K) = K mod N
    // Для i>0: рекурсивно вычисляем
    unsigned int hash = h0;
    for (unsigned int i = 0; i < iteration; ++i) {
      hash = static_cast<unsigned int>(std::floor(hash * kA * _tableSize)) %
             _tableSize;
    }
    return hash;
  }

  case HashType::Double:
    // hi(K) = ((K mod N) + i × (1 + K mod (N - 2))) mod N
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
template <typename T> HashTable<T>::~HashTable() {
  // Векторы и списки автоматически освобождают память
}

// Вставка элемента
template <typename T>
void HashTable<T>::insert(unsigned int const key, T const element) {
  // Проходим по всем возможным позициям
  for (unsigned int i = 0; i < _tableSize; ++i) {
    unsigned int currentIndex = _getHash(key, i);

    // Проверяем, есть ли элемент с таким ключом в списке
    for (auto &pair : _table[currentIndex]) {
      if (pair.first == key) {
        // Ключ найден, обновляем значение
        pair.second = element;
        return;
      }
    }

    // Элемента с таким ключом нет, добавляем в этот индекс
    _table[currentIndex].push_back({key, element});
    return;
  }

  // Если таблица переполнена (все ячейки проверены)
  std::cerr << "Hash table is full, cannot insert key " << key << std::endl;
}

// Удаление элемента по ключу
template <typename T> void HashTable<T>::remove(unsigned int const key) {
  for (unsigned int i = 0; i < _tableSize; ++i) {
    unsigned int currentIndex = _getHash(key, i);

    // Ищем элемент с данным ключом в списке
    for (auto it = _table[currentIndex].begin();
         it != _table[currentIndex].end(); ++it) {
      if (it->first == key) {
        _table[currentIndex].erase(it);
        return;
      }
    }

    // Если ячейка пуста, элемент не найден
    if (_table[currentIndex].empty()) {
      return;
    }
  }
}

// Проверка наличия элемента
template <typename T> bool HashTable<T>::exist(unsigned int const key) const {
  for (unsigned int i = 0; i < _tableSize; ++i) {
    unsigned int currentIndex = _getHash(key, i);

    // Ищем элемент с данным ключом в списке
    for (const auto &pair : _table[currentIndex]) {
      if (pair.first == key) {
        return true;
      }
    }

    // Если ячейка пуста, элемент не найден
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

// Изменение размера хеш-таблицы
template <typename T> void HashTable<T>::resize(size_t const size) {
  if (size == 0) {
    std::cerr << "Cannot resize to 0" << std::endl;
    return;
  }

  // Сохраняем старые данные
  std::vector<std::list<std::pair<unsigned int, T>>> oldTable = _table;
  size_t oldSize = _tableSize;

  // Создаём новую таблицу
  _tableSize = size;
  _table.clear();
  _table.resize(_tableSize);

  // Переносим все элементы
  for (size_t i = 0; i < oldSize; ++i) {
    for (const auto &pair : oldTable[i]) {
      insert(pair.first, pair.second);
    }
  }
}

// Пересчёт позиций элементов с текущей хеш-функцией
template <typename T> void HashTable<T>::rehash() {
  // Сохраняем все элементы
  std::vector<std::pair<unsigned int, T>> allElements;

  for (size_t i = 0; i < _tableSize; ++i) {
    for (const auto &pair : _table[i]) {
      allElements.push_back(pair);
    }
  }

  // Очищаем таблицу
  _table.clear();
  _table.resize(_tableSize);

  // Заново вставляем все элементы
  for (const auto &pair : allElements) {
    insert(pair.first, pair.second);
  }
}

// Замена хеш-функции
template <typename T>
void HashTable<T>::setHashFunction(const HashType newtype) {
  _currentType = newtype;
  rehash(); // Пересчитываем позиции всех элементов
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
  // Ищем элемент
  for (unsigned int i = 0; i < _tableSize; ++i) {
    unsigned int currentIndex = _getHash(key, i);

    for (auto &pair : _table[currentIndex]) {
      if (pair.first == key) {
        return pair.second;
      }
    }

    // Если нашли пустую ячейку, добавляем новый элемент
    if (_table[currentIndex].empty()) {
      _table[currentIndex].push_back({key, T()});
      return _table[currentIndex].back().second;
    }
  }

  // Если таблица заполнена, добавляем в последнюю позицию
  unsigned int lastIndex = _getHash(key, _tableSize - 1);
  _table[lastIndex].push_back({key, T()});
  return _table[lastIndex].back().second;
}

#endif // HASHTABLE
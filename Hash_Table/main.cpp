#include "hashtable.cpp"
#include <iostream>
#include <string>

int main() {
  std::cout << "=== Тестирование HashTable ===" << std::endl << std::endl;

  // Создание хеш-таблицы для строк
  HashTable<std::string> table(7);

  std::cout << "1. Добавление элементов:" << std::endl;
  table.insert(10, "Элемент 10");
  table.insert(20, "Элемент 20");
  table.insert(15, "Элемент 15");
  table.insert(7, "Элемент 7");
  table.insert(35, "Элемент 35");

  std::cout << "\nСодержимое таблицы:" << std::endl;
  table.print();

  // Проверка существования элементов
  std::cout << "\n2. Проверка существования элементов:" << std::endl;
  std::cout << "Ключ 20 существует: " << (table.exist(20) ? "Да" : "Нет")
            << std::endl;
  std::cout << "Ключ 99 существует: " << (table.exist(99) ? "Да" : "Нет")
            << std::endl;

  // Обращение через operator[]
  std::cout << "\n3. Доступ через operator[]:" << std::endl;
  std::cout << "table[20] = " << table[20] << std::endl;
  table[20] = "Обновленный элемент 20";
  std::cout << "После изменения: table[20] = " << table[20] << std::endl;

  // Удаление элемента
  std::cout << "\n4. Удаление элемента с ключом 15:" << std::endl;
  table.remove(15);
  std::cout << "Ключ 15 существует: " << (table.exist(15) ? "Да" : "Нет")
            << std::endl;
  table.print();

  // Тестирование различных хеш-функций
  std::cout << "\n5. Смена хеш-функции на Double:" << std::endl;
  table.setHashFunction(HashType::Double);
  table.print();

  std::cout << "\n6. Смена хеш-функции на Recursive:" << std::endl;
  table.setHashFunction(HashType::Recursive);
  table.print();

  std::cout << "\n7. Возврат к Quadratic:" << std::endl;
  table.setHashFunction(HashType::Quadratic);
  table.print();

  // Изменение размера таблицы
  std::cout << "\n8. Изменение размера таблицы на 13:" << std::endl;
  table.resize(13);
  table.print();

  // Тестирование конструктора копирования
  std::cout << "\n9. Создание копии таблицы:" << std::endl;
  HashTable<std::string> table2(table);
  std::cout << "Копия таблицы:" << std::endl;
  table2.print();

  // Тестирование swap
  std::cout << "\n10. Создание новой таблицы и swap:" << std::endl;
  HashTable<std::string> table3(5);
  table3.insert(100, "Новая таблица");
  table3.insert(200, "Элемент 200");

  std::cout << "До swap - table:" << std::endl;
  table.print();
  std::cout << "\nДо swap - table3:" << std::endl;
  table3.print();

  table.swap(table3);

  std::cout << "\nПосле swap - table:" << std::endl;
  table.print();
  std::cout << "\nПосле swap - table3:" << std::endl;
  table3.print();

  // Тестирование оператора присваивания
  std::cout << "\n11. Тестирование operator=:" << std::endl;
  HashTable<std::string> table4;
  table4 = table3;
  std::cout << "table4 после присваивания:" << std::endl;
  table4.print();

  // Тестирование с числовыми значениями
  std::cout << "\n12. Тестирование с числовыми значениями:" << std::endl;
  HashTable<int> numTable(5);
  numTable.insert(1, 100);
  numTable.insert(6, 200);
  numTable.insert(11, 300);
  numTable.insert(16, 400);

  std::cout << "Числовая таблица:" << std::endl;
  numTable.print();

  std::cout << "\n=== Тестирование завершено ===" << std::endl;

  return 0;
}
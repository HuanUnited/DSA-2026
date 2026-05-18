#include "hashtable.cpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== Testing HashTable ===" << std::endl << std::endl;

    HashTable<std::string> table(7);

    std::cout << "1. Adding elements:" << std::endl;
    table.insert(10, "Element 10");
    table.insert(20, "Element 20");
    table.insert(15, "Element 15");
    table.insert(7, "Element 7");
    table.insert(35, "Element 35");

    std::cout << "\nTable contents:" << std::endl;
    table.print();

    std::cout << "\n2. Checking the existence of elements:" << std::endl;
    std::cout << "Key 20 exists: " << (table.exist(20) ? "Yes" : "No")
            << std::endl;
    std::cout << "Key 99 exists: " << (table.exist(99) ? "Yes" : "No")
            << std::endl;

    std::cout << "\n3. Access using operator[]:" << std::endl;
    std::cout << "table[20] = " << table[20] << std::endl;
    table[20] = "Updating element 20";
    std::cout << "After change: table[20] = " << table[20] << std::endl;

    std::cout << "\n4. Deleting element with the key 15:" << std::endl;
    table.remove(15);
    std::cout << "Key 15 exists: " << (table.exist(15) ? "Yes" : "No")
            << std::endl;
    table.print();

    std::cout << "\n5. Double hash scheme:" << std::endl;
    DoubleHash doubleHash;
    table.setHashFunction(&doubleHash);
    table.print();

    std::cout << "\n6. Changing hash function to Recursive:" << std::endl;
    RecursiveHash recursiveHash;
    table.setHashFunction(&recursiveHash);
    table.print();

    std::cout << "\n7. Returning to Quadratic hash function:" << std::endl;
    QuadraticHash quadraticHash;
    table.setHashFunction(&quadraticHash);
    table.print();

    std::cout << "\n8. Changing the size of the table to 13:" << std::endl;
    table.resize(13);
    table.print();

    std::cout << "\n9. Making a copy of a table:" << std::endl;
    HashTable<std::string> table2(table);
    std::cout << "Copied table:" << std::endl;
    table2.print();

    std::cout << "\n10. Making a new table and swap:" << std::endl;
    HashTable<std::string> table3(5);
    table3.insert(100, "New table");
    table3.insert(200, "Element 200");

    std::cout << "Before swap - table:" << std::endl;
    table.print();
    std::cout << "\nBefore swap - table3:" << std::endl;
    table3.print();

    table.swap(table3);

    std::cout << "\nAfter swap - table:" << std::endl;
    table.print();
    std::cout << "\nAfter swap - table3:" << std::endl;
    table3.print();

    std::cout << "\n11. Testing operator=:" << std::endl;
    HashTable<std::string> table4 = table3;
    std::cout << "table4 after equal operator:" << std::endl;
    table4.print();

    std::cout << "\n12. Testing with number values:" << std::endl;
    HashTable<int> numTable(5);
    numTable.insert(1, 100);
    numTable.insert(6, 200);
    numTable.insert(11, 300);
    numTable.insert(16, 400);

    std::cout << "Number table:" << std::endl;
    numTable.print();

    std::cout << "\n=== Testing complete ===" << std::endl;

    return 0;
}
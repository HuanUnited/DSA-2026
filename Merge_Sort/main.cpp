#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

enum class MergeMode {
  Direct,
  Natural,
};

enum class SortAlgorithm {
  BalancedMultiway,
  Polyphase,
};

// ------- Помощникик --------
bool createFileWithRandomNumbers(const std::string &fileName,
                                 const int numbersCount,
                                 const int maxNumberValue) {
  std::ofstream outFile(fileName, std::ios::trunc);
  if (!outFile.is_open())
    return false;

  // Используем <random> для больших диапазонов и отрицательных чисел
  std::mt19937_64 rng(12345);
  std::uniform_int_distribution<int> dist(-maxNumberValue, maxNumberValue);

  for (int i = 0; i < numbersCount; ++i) {
    outFile << dist(rng) << (i == numbersCount - 1 ? "" : " ");
  }
  return true;
}

bool isFileContainsSortedArray(const std::string &fileName) {
  std::ifstream inFile(fileName);
  if (!inFile.is_open())
    return false;

  int current, previous;
  if (!(inFile >> previous))
    return true;

  while (inFile >> current) {
    if (current < previous)
      return false;
    previous = current;
  }
  return true;
}
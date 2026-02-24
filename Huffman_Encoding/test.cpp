/*
 * test.cpp
 * Проверки и тесты для кодирования Хаффмана.
 */

#include "huffman.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <string>

// --- тестовый фреймворк -----
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name)                                                             \
  static void test_##name();                                                   \
  static struct _reg_##name {                                                  \
    _reg_##name() {                                                            \
      ++tests_run;                                                             \
      std::cout << " [ RUN ] " #name "\n";                                     \
      try {                                                                    \
        test_##name();                                                         \
        ++tests_passed;                                                        \
        std::cout << " [ OK ] " #name "\n";                                    \
      } catch (const std::exception &e) {                                      \
        std::cout << " [ FAIL ] " #name " - " << e.what() << "\n";             \
      } catch (...) {                                                          \
        std::cout << " [ FAIL ] " #name " - Неопределенная ошибка \n";         \
      }                                                                        \
    }                                                                          \
  } _reg_##name##_instance;                                                    \
  void test_##name()

#define REQUIRE(cond)                                                          \
  do {                                                                         \
    if (!(cond))                                                               \
      throw std::runtime_error("REQUIRE failed:" #cond " (" __FILE__ ":" +     \
                               std::to_string(__LINE__) + ")");                \
  } while (0)

// --- Файловые помощники ---
static void writeTextFile(const std::string &path, const std::string &content) {
  std::ofstream f(path, std::ios::binary);
  f.write(content.data(), static_cast<std::streamsize>(content.size()));
}

static std::string readTextFile(const std::string &path) {
  std::fstream f(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(f), {}};
}

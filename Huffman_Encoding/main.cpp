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
  std::ifstream f(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(f), {}};
}

// ─────────────────────────────────────────────────────────────────────────────
//  1. BUILD
//     Does build() replace the old tree when called twice?
// ─────────────────────────────────────────────────────────────────────────────
TEST(build_replaces_old_tree) {
  HuffmanCoder h;

  // Build for "aaa", then encode
  h.build("aaa");
  std::string enc1, dec1;
  double r1 = h.encode("aaa", enc1);
  REQUIRE(r1 > 0);
  REQUIRE(h.decode(enc1, dec1) && dec1 == "aaa");

  // Rebuild for completely different alphabet
  h.build("xyz");
  std::string enc2, dec2;
  double r2 = h.encode("xyz", enc2);
  REQUIRE(r2 > 0);
  REQUIRE(h.decode(enc2, dec2) && dec2 == "xyz");

  // Old encoding must NOT decode correctly with new tree
  // (the trees are different, so the result should be wrong or fail)
  std::string bad;
  bool ok = h.decode(enc1, bad);
  REQUIRE(!ok || bad != "aaa");
}

// ─────────────────────────────────────────────────────────────────────────────
//  2. ENCODE (strings)
//     Basic round-trip
// ─────────────────────────────────────────────────────────────────────────────
TEST(encode_decode_basic_roundtrip) {
  HuffmanCoder h;
  const std::string original = "abracadabra";
  std::string encoded, decoded;

  double ratio = h.encode(original, encoded);
  REQUIRE(ratio > 0.0);

  // Encoded string must only contain '0' and '1'
  for (char c : encoded)
    REQUIRE(c == '0' || c == '1');

  bool ok = h.decode(encoded, decoded);
  REQUIRE(ok);
  REQUIRE(decoded == original);
}

// ─────────────────────────────────────────────────────────────────────────────
//  3. ENCODE — auto-build
//     If the tree is empty, encode() must build it automatically.
// ─────────────────────────────────────────────────────────────────────────────
TEST(encode_builds_tree_automatically) {
  HuffmanCoder h; // no build() called
  std::string enc, dec;
  double ratio = h.encode("hello", enc); // should trigger build internally
  REQUIRE(ratio > 0.0);
  REQUIRE(h.decode(enc, dec) && dec == "hello");
}

// ─────────────────────────────────────────────────────────────────────────────
//  4. ENCODE — compression ratio sanity
//     A highly skewed distribution should compress well (ratio > 1).
//     A single unique char gives ratio == original_bits / encoded_bits.
// ─────────────────────────────────────────────────────────────────────────────
TEST(encode_compression_ratio) {
  HuffmanCoder h;
  // "aaaaaabbc" — 'a' dominates, should compress
  std::string enc, dec;
  double ratio = h.encode("aaaaaabbc", enc);

  // ratio > 1 means we used fewer bits than the raw 8-bit encoding
  REQUIRE(ratio > 1.0);

  // Decode must still be correct
  REQUIRE(h.decode(enc, dec) && dec == "aaaaaabbc");
}

// ─────────────────────────────────────────────────────────────────────────────
//  5. ENCODE — returns -1 when encoding cannot proceed
//     Empty string → can't build a meaningful tree.
// ─────────────────────────────────────────────────────────────────────────────
TEST(encode_returns_minus1_on_empty_input) {
  HuffmanCoder h;
  std::string enc;
  double ratio = h.encode("", enc);
  REQUIRE(ratio == -1.0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  6. DECODE — returns false when tree is empty
// ─────────────────────────────────────────────────────────────────────────────
TEST(decode_fails_without_tree) {
  HuffmanCoder h; // no build, no encode
  std::string dec;
  bool ok = h.decode("010101", dec);
  REQUIRE(!ok);
}

// ─────────────────────────────────────────────────────────────────────────────
//  7. SINGLE unique character
//     "aaaaa" — only 'a' in the alphabet.
//     The tree has a single leaf; each encoded bit represents one 'a'.
// ─────────────────────────────────────────────────────────────────────────────
TEST(single_unique_character) {
  HuffmanCoder h;
  std::string enc, dec;

  double ratio = h.encode("aaaaa", enc);
  REQUIRE(ratio > 0.0);
  // Encoded length should equal number of characters (one bit per char)
  REQUIRE(enc.size() == 5);

  bool ok = h.decode(enc, dec);
  REQUIRE(ok && dec == "aaaaa");
}

// ─────────────────────────────────────────────────────────────────────────────
//  8. TWO unique characters
//     "aabb" — one bit per character is optimal.
// ─────────────────────────────────────────────────────────────────────────────
TEST(two_unique_characters) {
  HuffmanCoder h;
  std::string enc, dec;
  REQUIRE(h.encode("aabb", enc) > 0.0);
  // 4 chars × 1 bit = 4 bits encoded
  REQUIRE(enc.size() == 4);
  REQUIRE(h.decode(enc, dec) && dec == "aabb");
}

// ─────────────────────────────────────────────────────────────────────────────
//  9. ALL unique characters (worst case for compression)
//     Every character appears once → the tree is balanced → long codes.
//     We only verify round-trip correctness; ratio may be < 1.
// ─────────────────────────────────────────────────────────────────────────────
TEST(all_unique_characters) {
  HuffmanCoder h;
  const std::string original = "abcdefgh"; // 8 distinct chars
  std::string enc, dec;
  double ratio = h.encode(original, enc);
  REQUIRE(ratio > 0.0); // encoding happened (even if ratio < 1)
  REQUIRE(h.decode(enc, dec) && dec == original);
}

// ─────────────────────────────────────────────────────────────────────────────
//  10. FULL ASCII range
//      256 distinct byte values. Also tests that non-printable bytes survive.
// ─────────────────────────────────────────────────────────────────────────────
TEST(full_ascii_range) {
  std::string original;
  original.reserve(256);
  for (int i = 0; i < 256; ++i)
    original += static_cast<char>(i);

  HuffmanCoder h;
  std::string enc, dec;
  double ratio = h.encode(original, enc);
  REQUIRE(ratio > 0.0);
  REQUIRE(h.decode(enc, dec) && dec == original);
}

// ─────────────────────────────────────────────────────────────────────────────
//  11. SINGLE CHARACTER string (length 1)
// ─────────────────────────────────────────────────────────────────────────────
TEST(single_character_string) {
  HuffmanCoder h;
  std::string enc, dec;
  REQUIRE(h.encode("z", enc) > 0.0);
  REQUIRE(h.decode(enc, dec) && dec == "z");
}

// ─────────────────────────────────────────────────────────────────────────────
//  12. PREFIX-FREE property
//      No code in the table should be a prefix of another.
//      We test this indirectly: if a random mixed-up bit string produces
//      garbage on decode, the codes were prefix-free.
//      Direct test: decode(encode(x)) == x proves no ambiguity existed.
// ─────────────────────────────────────────────────────────────────────────────
TEST(prefix_free_property) {
  HuffmanCoder h;
  const std::string original = "the quick brown fox jumps over the lazy dog";
  std::string enc, dec;
  REQUIRE(h.encode(original, enc) > 0.0);
  REQUIRE(h.decode(enc, dec));
  // If prefix-free was violated, decoded text would differ
  REQUIRE(dec == original);
}

// ─────────────────────────────────────────────────────────────────────────────
//  13. ENCODE FILE / DECODE FILE — basic round-trip
// ─────────────────────────────────────────────────────────────────────────────
TEST(file_encode_decode_roundtrip) {
  const std::string original = "hello huffman world";
  writeTextFile("./huf_in.bin", original);

  HuffmanCoder h;
  double ratio =
      h.encodeFile(std::string("./huf_in.bin"), std::string("./huf_enc.bin"));
  REQUIRE(ratio > 0.0);

  bool ok =
      h.decodeFile(std::string("./huf_enc.bin"), std::string("./huf_dec.bin"));
  REQUIRE(ok);
  REQUIRE(readTextFile("./huf_dec.bin") == original);
}

// ─────────────────────────────────────────────────────────────────────────────
//  14. ENCODE FILE — compression ratio is better than raw for skewed input
// ─────────────────────────────────────────────────────────────────────────────
TEST(file_compression_ratio) {
  // 1000 'a's — maximally skewed
  const std::string original(1000, 'a');
  writeTextFile("./huf_skew.bin", original);

  HuffmanCoder h;
  double ratio = h.encodeFile(std::string("./huf_skew.bin"),
                              std::string("./huf_skew_enc.bin"));
  // 1000 chars × 8 bits raw / 1000 × 1 bit encoded = 8
  REQUIRE(ratio >= 7.0); // leave some slack for the header overhead
}

// ─────────────────────────────────────────────────────────────────────────────
//  15. ENCODE FILE — returns -1 when file does not exist
// ─────────────────────────────────────────────────────────────────────────────
TEST(encodeFile_returns_minus1_on_missing_file) {
  HuffmanCoder h;
  double ratio = h.encodeFile(std::string("./does_not_exist_xyz.bin"),
                              std::string("./out.bin"));
  REQUIRE(ratio == -1.0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  16. DECODE FILE — returns false when file does not exist
// ─────────────────────────────────────────────────────────────────────────────
TEST(decodeFile_returns_false_on_missing_file) {
  HuffmanCoder h;
  bool ok = h.decodeFile(std::string("./does_not_exist_xyz.bin"),
                         std::string("./out.bin"));
  REQUIRE(!ok);
}

// ─────────────────────────────────────────────────────────────────────────────
//  17. FILE — single unique character
// ─────────────────────────────────────────────────────────────────────────────
TEST(file_single_unique_char) {
  const std::string original(20, 'x');
  writeTextFile("./huf_singlechar.bin", original);

  HuffmanCoder h;
  double ratio = h.encodeFile(std::string("./huf_singlechar.bin"),
                              std::string("./huf_singlechar_enc.bin"));
  REQUIRE(ratio > 0.0);

  bool ok = h.decodeFile(std::string("./huf_singlechar_enc.bin"),
                         std::string("./huf_singlechar_dec.bin"));
  REQUIRE(ok);
  REQUIRE(readTextFile("./huf_singlechar_dec.bin") == original);
}

// ─────────────────────────────────────────────────────────────────────────────
//  18. FILE — binary (non-text) content survives round-trip
// ─────────────────────────────────────────────────────────────────────────────
TEST(file_binary_content) {
  std::string original;
  original.reserve(256);
  for (int i = 0; i < 256; ++i)
    original += static_cast<char>(i);

  writeTextFile("./huf_bin.bin", original);

  HuffmanCoder h;
  REQUIRE(h.encodeFile(std::string("./huf_bin.bin"),
                       std::string("./huf_bin_enc.bin")) > 0.0);
  REQUIRE(h.decodeFile(std::string("./huf_bin_enc.bin"),
                       std::string("./huf_bin_dec.bin")));
  REQUIRE(readTextFile("./huf_bin_dec.bin") == original);
}

// ─────────────────────────────────────────────────────────────────────────────
//  19. EXPORT / IMPORT TREE — decode using a tree loaded from disk
// ─────────────────────────────────────────────────────────────────────────────
TEST(export_import_tree) {
  const std::string original = "export and import test string 12345";
  writeTextFile("./huf_exp.bin", original);

  // Encoder builds and saves the tree
  HuffmanCoder encoder;
  REQUIRE(encoder.encodeFile(std::string("./huf_exp.bin"),
                             std::string("./huf_exp_enc.bin")) > 0.0);
  encoder.exportTree(std::string("./huf_tree.bin"));

  // Decoder loads tree from disk (knows nothing about original text)
  HuffmanCoder decoder;
  decoder.importTree(std::string("./huf_tree.bin"));
  REQUIRE(decoder.decodeFile(std::string("./huf_exp_enc.bin"),
                             std::string("./huf_exp_dec.bin")));
  REQUIRE(readTextFile("./huf_exp_dec.bin") == original);
}

// ─────────────────────────────────────────────────────────────────────────────
//  20. EXPORT TREE — throws when tree is empty
// ─────────────────────────────────────────────────────────────────────────────
TEST(exportTree_throws_when_empty) {
  HuffmanCoder h;
  bool threw = false;
  try {
    h.exportTree(std::string("./huf_empty_tree.bin"));
  } catch (const std::runtime_error &) {
    threw = true;
  }
  REQUIRE(threw);
}

// ─────────────────────────────────────────────────────────────────────────────
//  21. IMPORT TREE — throws when file does not exist
// ─────────────────────────────────────────────────────────────────────────────
TEST(importTree_throws_on_missing_file) {
  HuffmanCoder h;
  bool threw = false;
  try {
    h.importTree(std::string("./does_not_exist_tree.bin"));
  } catch (const std::runtime_error &) {
    threw = true;
  }
  REQUIRE(threw);
}

// ─────────────────────────────────────────────────────────────────────────────
//  22. MULTIPLE ENCODE calls on the same coder
//      After encoding "abc", encoding a different string with the same coder
//      (no rebuild) must still work as long as all characters were seen.
// ─────────────────────────────────────────────────────────────────────────────
TEST(multiple_encodes_same_coder) {
  HuffmanCoder h;
  h.build("abcde");

  for (const std::string &s : {"ace", "bde", "abcde", "a", "e"}) {
    std::string enc, dec;
    double ratio = h.encode(s, enc);
    REQUIRE(ratio > 0.0);
    REQUIRE(h.decode(enc, dec) && dec == s);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  23. LARGE FILE — 100 KB of repeating text
// ─────────────────────────────────────────────────────────────────────────────
TEST(large_file_roundtrip) {
  // Build 100 KB from a short repeating pattern
  std::string pattern = "the quick brown fox jumps over the lazy dog\n";
  std::string original;
  original.reserve(100 * 1024);
  while (original.size() < 100 * 1024)
    original += pattern;
  original.resize(100 * 1024);

  writeTextFile("./huf_large.bin", original);

  HuffmanCoder h;
  double ratio = h.encodeFile(std::string("./huf_large.bin"),
                              std::string("./huf_large_enc.bin"));
  REQUIRE(ratio > 1.0); // repetition should compress well

  REQUIRE(h.decodeFile(std::string("./huf_large_enc.bin"),
                       std::string("./huf_large_dec.bin")));
  REQUIRE(readTextFile("./huf_large_dec.bin") == original);
}

int main() {
  std::cout << "\n=== Huffman Coder Tests ===\n\n";
  std::cout << "\n"
            << tests_passed << " / " << tests_run << " tests passed.\n\n";
  return (tests_passed == tests_run) ? 0 : 1;
}

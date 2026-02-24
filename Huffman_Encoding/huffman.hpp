/*
 * huffman.hpp — Алгоритм Хаффмана (20-балльный вариант)
 *
 * Требования:
 *   - входные данные и результат — файлы
 *   - коды хранятся в виде упакованных бит (BitVector)
 *   - пути в узлах дерева — BitVector
 *   - методы exportTree / importTree
 *
 * Зависит от: bitvector.hpp
 */

#ifndef HUFFMAN
#define HUFFMAN

#include "bitvector.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// ----------------------------
//  Вспомогательные функция: упоковать/распаковать байт поток <-> BitVector
// ----------------------------

namespace huffman_detail {

inline void writeBits(std::ostream &out, const BitVector &bv) {
  size_t nbits = bv.size();
  size_t nbytes = BitVector::bytes_for_bits(nbits);
  for (size_t b = 0; b < nbytes; ++b) {
    uint8_t byte = 0;
    for (size_t bit = 0; bit < 8 && b * 8 + bit < nbits; ++bit) {
      if (bv[b * 8 + bit])
        byte |= static_cast<uint8_t>(1u << bit);
    }
    out.write(reinterpret_cast<const char *>(&byte), 1);
  }
}

inline BitVector readBits(std::istream &in, size_t clen) {
  BitVector bv(clen, false);
  size_t nbytes = BitVector::bytes_for_bits(clen);
  for (size_t b = 0; b < nbytes; ++b) {
    uint8_t byte = 0;
    in.read(reinterpret_cast<char *>(&byte), 1);
    for (size_t bit = 0; bit < 8 && b * 8 + bit < clen; ++bit)
      if (byte & (1u << bit))
        bv.set(b * 8 + bit, true);
  }
  return bv;
}

} // namespace huffman_detail

/**
 * @class HuffmanCoder
 * @brief Класс для кодирования Хаффмана
 */
class HuffmanCoder {
private:
  /**
   * @struct Node
   * @brief Узел для дерева Хаффмана.
   */
  struct Node {
    int _ch;
    size_t _freq;
    BitVector _code;
    std::unique_ptr<Node> _left, _right;

    Node(int c, size_t f) : _ch(c), _freq(f) {};
    Node(size_t f, std::unique_ptr<Node> l, std::unique_ptr<Node> r)
        : _ch(-1), _freq(f), _left(std::move(l)), _right(std::move(r)) {};
    bool isLeaf() const { return !_left && !_right; };
  };

  // ------- Данные Класса -----------
  std::unique_ptr<Node> _root;
  std::unordered_map<int, BitVector> _codeTable;

  // ------- Построение Кодов --------
  void _buildCodes(Node *node, BitVector prefix) {
    if (!node)
      return;

    if (node->isLeaf()) {
      if (prefix.size() == 0) {
        prefix = BitVector(1, false);
      }
      node->_code = prefix;
      _codeTable[node->_ch] = prefix;
      return;
    }

    // Влево - добавляем 0 ;
    BitVector lcode(prefix.size() + 1, false);
    for (size_t i = 0; i < prefix.size(); ++i)
      lcode.set(i, prefix[i]);
    lcode.set(prefix.size(), false);
    _buildCodes(node->_left.get(), lcode);

    // Вправо - добавляем 1 ;
    BitVector rcode(prefix.size() + 1, false);
    for (size_t i = 0; i < prefix.size(); ++i)
      rcode.set(i, prefix[i]);
    rcode.set(prefix.size(), true);
    _buildCodes(node->_right.get(), rcode);
  }

  // -------- Восстановление дерева из таблицы кодов ------------
  void _rebuidFromTable(const std::unordered_map<int, BitVector> &table) {
    _root = std::make_unique<Node>(-1, 0);
    _codeTable.clear();

    for (auto &[ch, code] : table) {
      Node *cur = _root.get();
      for (size_t i = 0; i < code.size(); ++i) {
        bool bit = code[i];
        if (!bit) {
          if (!cur->_left)
            cur->_left = std::make_unique<Node>(-1, 0);
          cur = cur->_left.get();
        } else {
          if (!cur->_right)
            cur->_right = std::make_unique<Node>(-1, 0);
          cur = cur->_right.get();
        }
      }
      cur->_ch = ch;
      const_cast<std::unordered_map<int, BitVector> &>(_codeTable)[ch] = code;
    }
    _codeTable = table;
  }

  // ---- Построение дерева из таблицы частот ----------------
  void _buildFromFreq(const std::unordered_map<int, size_t> &_freq) {
    _root.reset();
    _codeTable.clear();
    if (_freq.empty())
      return;

    std::vector<std::unique_ptr<Node>> store;
    store.reserve(_freq.size());
    for (auto &[ch, f] : _freq)
      store.push_back(std::make_unique<Node>(ch, f));

    auto cmp = [](const Node *a, const Node *b) { return a->_freq > b->_freq; };
    std::vector<Node *> heap;
    heap.reserve(store.size());
    for (auto &n : store)
      heap.push_back(n.get());

    auto heapPush = [&](Node *n) {
      heap.push_back(n);
      std::push_heap(heap.begin(), heap.end(), cmp);
    };

    auto heapPop = [&]() -> Node * {
      std::pop_heap(heap.begin(), heap.end(), cmp);
      Node *n = heap.back();
      heap.pop_back();
      return n;
    };

    std::make_heap(heap.begin(), heap.end(), cmp);

    while (heap.size() > 1) {
      Node *a = heapPop();
      Node *b = heapPop();

      std::unique_ptr<Node> ua, ub;
      for (auto &s : store)
        if (s.get() == a) {
          ua = std::move(s);
          break;
        }
      for (auto &s : store)
        if (s.get() == b) {
          ub = std::move(s);
          break;
        }

      auto parent = std::make_unique<Node>(ua->_freq + ub->_freq, std::move(ua),
                                           std::move(ub));
      heapPush(parent.get());
      store.push_back(std::move(parent));
    }

    for (auto &s : store)
      if (s) {
        _root = std::move(s);
        break;
      }

    _buildCodes(_root.get(), BitVector());
  }

public:
  HuffmanCoder() = default;

  /**
   * @param text
   * @brief построить дерево из строки (старое дерево удаляется)
   */
  void build(const std::string &text) {
    std::unordered_map<int, size_t> freq;
    for (unsigned char c : text)
      ++freq[c];
    _buildFromFreq(freq);
  }

  /**
   * @param input
   * @param output
   * @brief Возращает коэффициент сжатия (original_bits / compressed_bits) или
   * -1
   */
  double encode(const std::string &input, std::string &output) {
    if (!_root)
      build(input);
    if (_codeTable.empty())
      return -1.0;

    output.clear();
    for (unsigned char c : input) {
      auto it = _codeTable.find(c);
      if (it == _codeTable.end())
        return -1.0;
      const BitVector &code = it->second;
      for (size_t i = 0; i < code.size(); ++i)
        output += (code[i] ? '1' : '0');
    }

    if (output.empty())
      return -1.0;
    return static_cast<double>(input.size() * 8) /
           static_cast<double>(output.size());
  }

  /**
   * @param encoded
   * @param output
   * @brief
   */
  bool decode(const std::string &encoded, std::string &output) {
    if (!_root)
      return false;
    output.clear();

    if (_root->isLeaf()) {
      for (char c : encoded) {
        if (c != '0' && c != '1')
          return false;
        output += static_cast<char>(_root->_ch);
      }
      return true;
    }

    Node *cur = _root.get();
    for (char c : encoded) {
      if (c == '0')
        cur = cur->_left.get();
      else if (c == '1')
        cur = cur->_right.get();
      else
        return false;
      if (!cur)
        return false;
      if (cur->isLeaf()) {
        output += static_cast<char>(cur->_ch);
        cur = _root.get();
      }
    }
    return cur == _root.get();
  }

  double encodeFile(const std::string &inputFile, std::string &outputFile) {
    std::ifstream in(inputFile, std::ios::binary);
    if (!in)
      return -1.0;
    const std::string text((std::istreambuf_iterator<char>(in)),
                           std::istreambuf_iterator<char>());
    in.close();
    if (text.empty())
      return -1.0;

    if (!_root)
      build(text);
    if (_codeTable.empty())
      return -1.0;

    size_t totalBits = 0;
    for (unsigned char c : text) {
      auto it = _codeTable.find(c);
      if (it == _codeTable.end())
        return -1.0;
      totalBits += it->second.size();
    }

    BitVector encoded(totalBits, false);
    size_t pos = 0;
    for (unsigned char c : text) {
      const BitVector &code = _codeTable[c];
      for (size_t i = 0; i < code.size(); ++i) {
        encoded.set(pos++, code[i]);
      }
    }

    std::ofstream out(outputFile, std::ios::binary);
    if (!out)
      return -1.0;

    uint32_t numChars = static_cast<uint32_t>(_codeTable.size());
    out.write(reinterpret_cast<const char *>(&numChars), sizeof(numChars));

    for (auto &[ch, code] : _codeTable) {
      uint8_t sym = static_cast<uint8_t>(ch);
      uint8_t clen = static_cast<uint8_t>(code.size());
      out.write(reinterpret_cast<const char *>(&sym), 1);
      out.write(reinterpret_cast<const char *>(&clen), 1);
      huffman_detail::writeBits(out, code);
    }

    uint64_t bitCount = static_cast<uint64_t>(totalBits);
    out.write(reinterpret_cast<const char *>(&bitCount), sizeof(bitCount));
    huffman_detail::writeBits(out, encoded);

    out.close();
    return (static_cast<double>(text.size()) * 8) /
           static_cast<double>(totalBits);
  }

  bool decodeFile(const std::string &inputFile, const std::string &outputFile) {
    std::fstream in(inputFile, std::ios::binary);
    if (!in)
      return false;

    uint32_t numChars = 0;
    if (!in.read(reinterpret_cast<char *>(&numChars), sizeof(numChars)))
      ;
    return false;

    std::unordered_map<int, BitVector> table;
    for (uint32_t i = 0; i < numChars; ++i) {
      uint8_t sym = 0, clen = 0;
      if (!in.read(reinterpret_cast<char *>(&sym), 1))
        return false;
      if (!in.read(reinterpret_cast<char *>(&clen), 1))
        return false;
      table[sym] = huffman_detail::readBits(in, clen);
    }

    _rebuidFromTable(table);

    uint64_t bitCount = 0;
    if (!in.read(reinterpret_cast<char *>(&bitCount), sizeof(bitCount)))
      return false;

    BitVector encoded =
        huffman_detail::readBits(in, static_cast<size_t>(bitCount));
    in.close();

    std::ofstream out(outputFile, std::ios::binary);
    if (!out)
      return false;
    if (!_root)
      return false;
    Node *cur = _root.get();

    if (_root->isLeaf()) {
      char ch = static_cast<char>(_root->_ch);
      for (size_t i = 0; i < bitCount; ++i)
        out.write(&ch, 1);
      out.close();
      return true;
    }

    for (size_t i = 0; i < bitCount; ++i) {
      bool bit = encoded[i];
      cur = bit ? cur->_right.get() : cur->_left.get();
      if (!cur)
        return false;
      if (cur->isLeaf()) {
        char ch = static_cast<char>(cur->_ch);
        out.write(&ch, 1);
        cur = _root.get();
      }
    }
    out.close();
    return cur == _root.get();
  }

  void exportTree(const std::string &filename) const {
    if (!_root)
      throw std::runtime_error("exportTree: Дерево не построено");
    std::ofstream out(filename, std::ios::binary);
    if (!out)
      throw std::runtime_error("exportTree: не удалось открыть " + filename);

    uint32_t numChars = static_cast<uint32_t>(_codeTable.size());
    out.write(reinterpret_cast<const char *>(&numChars), 1);

    for (auto &[ch, code] : _codeTable) {
      uint8_t sym = static_cast<uint8_t>(ch);
      uint8_t clen = static_cast<uint8_t>(code.size());
      out.write(reinterpret_cast<const char *>(&sym), 1);
      out.write(reinterpret_cast<const char *>(&sym), 1);
      huffman_detail::writeBits(out, code);
    }
  }

  void importTree(const std::string &filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in)
      throw std::runtime_error("importTree: не удалось открыть " + filename);

    uint32_t numChars = 0;
    if (!in.read(reinterpret_cast<char *>(&numChars), sizeof(numChars)))
      throw std::runtime_error("importTree: неверный формат файла");

    std::unordered_map<int, BitVector> table;
    for (uint32_t i = 0; i < numChars; ++i) {
      uint8_t sym = 0, clen = 0;
      in.read(reinterpret_cast<char *>(&sym), 1);
      in.read(reinterpret_cast<char *>(&clen), 1);
      table[sym] = huffman_detail::readBits(in, clen);
    }
    _rebuidFromTable(table);
  }
};

#endif // HUFFMAN

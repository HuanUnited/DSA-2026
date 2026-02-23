#ifndef BITVECTOR
#define BITVECTOR

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using byte_t = uint8_t;
using pair = std::pair<size_t, size_t>;
static constexpr size_t BITS_PER_BYTE = CHAR_BIT; // = 8.

inline size_t max(size_t a, size_t b) { return (a >= b ? a : b); }

class BitVector {
public:
  // Byte to Bit converter
  static size_t bytes_for_bits(size_t bits) {
    return (bits + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
  }

  // Clean garbage bits and preserve valid bits (т.е: Valid bit filter.)
  byte_t last_byte_mask() const {
    size_t rem =
        nbits % BITS_PER_BYTE; // remainder bits (valid bits in the last byte)
    if (rem == 0)
      return static_cast<byte_t>(~byte_t(0)); // returns all 1 mask = 11111111
    return static_cast<byte_t>((1u << rem) -
                               1u); // returns a mask with all 1s in the lowest
                                    // bit eg 1u << 3 - 1 = 00001111
  }

  // Range Checker
  void check_index(size_t i) const {
    if (i >= nbits)
      throw std::out_of_range("Index");
  }

private:
  std::unique_ptr<byte_t[]> data;
  size_t nbits = 0;

  // TODO: Class in Class, interesting.
public:
  // BoolRef proxy: supports bv[i] = bool; and bool(bv[i])
  class BoolRef {
    BitVector &bv;
    size_t idx;

  public:
    BoolRef(BitVector &parent, size_t i) : bv(parent), idx(i) {}
    // assign a bool
    BoolRef &operator=(bool val) {
      bv.set(idx, val);
      return *this;
    }
    // assign from another BoolRef
    BoolRef &operator=(const BoolRef &other) {
      return (*this = static_cast<bool>(other));
    }
    // convert to bool for reading
    operator bool() const { return bv.get(idx); }
    // flip convenience
    BoolRef &flip() {
      bv.flip(idx);
      return *this;
    }
  };

  // -- Constructors / Destructor / Assignment --

  // По умолчанию
  BitVector() = default;

  // С параметрами (размер и значение - одно и то же для всех разрядов)
  BitVector(size_t size, bool value = false) : nbits(size) {
    size_t nbytes = bytes_for_bits(size);
    if (nbytes) {
      data = std::make_unique<byte_t[]>(nbytes);
      std::fill_n(data.get(), nbytes, value ? byte_t(0xFF) : byte_t(0x00));
      if (value && (nbits % BITS_PER_BYTE != 0))
        data[nbytes - 1] &= last_byte_mask();
    }
  }

  // Конструктор из массива const char *
  explicit BitVector(const char *bitstr) {
    if (!bitstr)
      throw std::invalid_argument("Null string");
    nbits = std::strlen(bitstr);
    size_t nbytes = bytes_for_bits(nbits);
    if (nbytes) {
      data = std::make_unique<byte_t[]>(nbytes);
      std::fill_n(data.get(), nbytes, byte_t(0x00));
      for (size_t i = 0; i < nbits; ++i) {
        char c = bitstr[i];
        if (c != '0' && c != '1')
          throw std::invalid_argument("Bit string must be '0' or '1'");
        if (c == '1')
          data[i / BITS_PER_BYTE] |=
              static_cast<byte_t>(1u << (i % BITS_PER_BYTE));
      }
    }
  }

  // Конструктор копирования
  BitVector(const BitVector &other) : nbits(other.nbits) {
    size_t nbytes = bytes_for_bits(nbits);
    if (nbytes) {
      data = std::make_unique<byte_t[]>(nbytes);
      std::memcpy(data.get(), other.data.get(), nbytes);
      // Other option in case no standard library.
      // for(size_t i = 0; i < nbits; ++i){
      //   data[i] = other.data[i];
      // }
    }
  }

  // Присваивание =
  BitVector &operator=(const BitVector &other) {
    if (this == &other)
      return *this;
    size_t nbytes = bytes_for_bits(other.nbits);
    if (nbytes) {
      auto tmp = std::unique_ptr<byte_t[]>(new byte_t[nbytes]);
      std::memcpy(tmp.get(), other.data.get(), nbytes);
      // Other option in case no standard library.
      // for(size_t i = 0; i < nbits; ++i){
      //   data[i] = other.data[i];
      // }
      data.swap(tmp); // unique_ptr swap.
    } else {
      data.reset(); // unique_ptr = nullptr.
    }
    nbits = other.nbits;
    return *this;
    // No need for delete temp[], unique ptr deletes itself.
  }

  BitVector(BitVector &&) noexcept = default;
  BitVector &operator=(BitVector &&) noexcept = default;
  ~BitVector() = default;

  // position: byte and offset:
  inline pair coord(size_t i) const {
    size_t b = i / BITS_PER_BYTE;   // which byte
    size_t off = i % BITS_PER_BYTE; // offset
    return {b, off};
  }

  // swap
  void swap(BitVector &other) noexcept {
    data.swap(other.data);
    std::swap(nbits, other.nbits);
  }
  // outside swap function.
  friend void swap(BitVector &a, BitVector &b) noexcept { a.swap(b); }

  // длина (количество бит) вектора;
  size_t size() const noexcept { return nbits; }

  // Получение компоненты подразумевается только на чтение
  bool get(size_t i) const {
    // boundary check
    check_index(i);
    pair position = coord(i); // abstraction is stupid
    // size_t b = i / BITS_PER_BYTE; // find out which byte contains the bit
    // size_t off = i % BITS_PER_BYTE; // find the bit offset in the byte.
    return (data[position.first] >> position.second) &
           1u; // shifts right by the offset to move the required bit to the
               // least significant pos, masks the bit with 1u (00000001) then
               // returns the bit as either true or false.
  }

  // Установка в 0/1 в i-той позиции
  void set(size_t i, bool value) {
    check_index(i);
    // size_t b = i / BITS_PER_BYTE; // which byte
    // size_t off = i % BITS_PER_BYTE; // offset

    pair b = coord(i);

    // TODO: Need explanation here.
    byte_t mask = static_cast<byte_t>(
        1u << b.second); // create a mask at the position of the target bit.
    if (value) // if the value is true, then use the OR operation to set the
               // bit at that position to true.
      data[b.first] |= mask;
    else // if it is false create an inverted mask say iu << 2 = 00000100 ~mask
         // = 11111011 and use the AND operator to force that bit to turn off.
      data[b.first] &= static_cast<byte_t>(~mask);
  }

  // flip single bit (invert i-th)
  void flip(size_t i) {
    // TODO: Here are some repeating actions for the same function, can we not
    // repeat ourselves?
    check_index(i);
    // size_t b = i / BITS_PER_BYTE; // which byte
    // size_t off = i % BITS_PER_BYTE; // offset
    pair b = coord(i);
    // TODO: Another explanation here.
    data[b.first] ^= static_cast<byte_t>(
        1u << b.second); // creates a mask at the offset position, then uses
                         // the XOR operator to flip.
  }

  // flip all bits
  void flipAll() {
    // TODO: Is flipping and inverting the same thing?
    size_t nbytes = bytes_for_bits(nbits);
    for (size_t i = 0; i < nbytes; ++i)
      data[i] = static_cast<byte_t>(~data[i]);
    if (nbytes && (nbits % BITS_PER_BYTE != 0))
      data[nbytes - 1] &= last_byte_mask();
  }

  // Установка в 0/1 в диапазоне;
  void setRange(size_t i, size_t k, bool value) {
    if (k == 0)
      return;
    if (i + k > nbits)
      throw std::out_of_range("Range out of bounds");
    // already optimized
    for (size_t pos = i; pos < i + k; ++pos)
      set(pos, value);
  }

  // Установка в 0/1 всех компонент вектора;
  void setAll(bool value) {
    size_t nbytes = bytes_for_bits(nbits);
    if (nbytes == 0)
      return;
    for (size_t pos = 0; pos < nbits; ++pos)
      set(pos, value); // operates bitwise.
    // std::fill_n(data.get(), nbytes, value ? byte_t(0xFF) : byte_t(0x00));
    // magic that makes things work properly. -> clears all unused bits in the
    // last byte to zero.
    if (value && (nbits % BITS_PER_BYTE != 0))
      data[nbytes - 1] &= last_byte_mask();
  }

  // Вес вектора (количество единичных компонент).
  size_t weight() const {
    size_t cnt = 0;
    // size_t nbytes = bytes_for_bits(nbits);
    // for (size_t i = 0; i < nbytes; ++i) {
    //   cnt += __builtin_popcount(static_cast<unsigned int>(data[i]));
    // }
    // // subtract bits in spare area if any (they should be cleared on
    // operations,
    // // but safe)
    // // TODO: What does this do and why.
    // if (nbytes && (nbits % BITS_PER_BYTE != 0)) {
    //   byte_t mask = last_byte_mask();
    //   byte_t spare = static_cast<byte_t>(data[nbytes - 1] & ~mask);
    //   if (spare) cnt -= __builtin_popcount(static_cast<unsigned int>(spare));
    // }

    for (size_t i = 0; i < nbits; ++i) {
      if (get(i)) {
        cnt++;
      }
    }

    return cnt;
  }

  // operator[] overloads
  BoolRef operator[](size_t i) {
    check_index(i);
    return BoolRef(*this, i);
  }

  // Получение компоненты ([ ], см. примечание ниже);
  bool operator[](size_t i) const { return get(i); }

  bool operator==(BitVector &rhs) const {
    if (nbits != rhs.nbits) {
      return false;
    }
    // non optimal solution
    for (size_t i = 0; i < nbits; i++) {
      if (get(i) != rhs.get(i))
        return false;
    }
    return true;
  }

  // Bitwise operators: &, |, ^ and their compound forms
  BitVector operator&(const BitVector &rhs) const {
    // if (nbits != rhs.nbits) {
    //   throw std::invalid_argument("Sizes must match for &");
    // }
    BitVector out(*this);
    size_t nbytes = bytes_for_bits(max(nbits, rhs.nbits));
    for (size_t i = 0; i < nbytes; ++i)
      out.data[i] &= rhs.data[i];
    if (nbytes && (max(nbits, rhs.nbits) % BITS_PER_BYTE != 0))
      out.data[nbytes - 1] &= out.last_byte_mask();
    return out;
  }
  BitVector &operator&=(const BitVector &rhs) {
    *this = *this & rhs;
    return *this;
  }

  BitVector operator|(const BitVector &rhs) const {
    // if (nbits != rhs.nbits)
    //   throw std::invalid_argument("Sizes must match for |");
    BitVector out(*this);
    size_t nbytes = bytes_for_bits(max(nbits, rhs.nbits));
    for (size_t i = 0; i < nbytes; ++i)
      out.data[i] |= rhs.data[i];
    if (nbytes && (max(nbits, rhs.nbits) % BITS_PER_BYTE != 0))
      out.data[nbytes - 1] &= out.last_byte_mask();
    return out;
  }
  BitVector &operator|=(const BitVector &rhs) {
    *this = *this | rhs;
    return *this;
  }

  BitVector operator^(const BitVector &rhs) const {
    // if (nbits != rhs.nbits)
    // throw std::invalid_argument("Sizes must match for ^");
    BitVector out(*this);
    size_t nbytes = bytes_for_bits(max(nbits, rhs.nbits));
    for (size_t i = 0; i < nbytes; ++i)
      out.data[i] ^= rhs.data[i];
    if (nbytes && (max(nbits, rhs.nbits) % BITS_PER_BYTE != 0))
      out.data[nbytes - 1] &= out.last_byte_mask();
    return out;
  }
  BitVector &operator^=(const BitVector &rhs) {
    *this = *this ^ rhs;
    return *this;
  }

  // bitwise NOT
  BitVector operator~() const {
    BitVector out(*this);
    size_t nbytes = bytes_for_bits(nbits);
    for (size_t i = 0; i < nbytes; ++i)
      out.data[i] = static_cast<byte_t>(~out.data[i]);
    if (nbytes && (nbits % BITS_PER_BYTE != 0))
      out.data[nbytes - 1] &= out.last_byte_mask();
    return out;
  }

  // shift operators
  // Left Shift
  BitVector operator<<(const size_t off) {
    if (off >= nbits)
      return BitVector(nbits, false);

    BitVector out(nbits, false);
    for (size_t i = 0; i < nbits - off; ++i) {
      if (get(i + off)) {
        out.set(i, true);
      }
    }
    return out;
  }
  BitVector operator<<=(const size_t off) {
    *this = *this << off;
    return *this;
  }

  // Right Shift
  BitVector operator>>(const size_t off) {
    if (off >= nbits)
      return BitVector(nbits, false);

    BitVector out(nbits, false);
    for (size_t i = 0; i < nbits - off; ++i) {
      if (get(i)) {
        out.set(i + off, true);
      }
    }
    return out;
  }
  BitVector operator>>=(const size_t off) {
    *this = *this >> off;
    return *this;
  }
};

inline std::ostream &operator<<(std::ostream &os, const BitVector &bv) {
  for (size_t i = 0; i < bv.size(); ++i)
    os << (bv[i] ? '1' : '0');
  return os;
}

inline std::istream &operator>>(std::istream &is, BitVector &bv) {
  std::string s;
  if (!(is >> s))
    return is;

  // Checks if the input is correct
  for (char c : s)
    if (c != '0' && c != '1') {
      is.setstate(std::ios::failbit);
      return is;
    }

  // Creates a bitvector with the length of the string, then sets all values
  // equal to 1 to true within the vector.
  bv = BitVector(s.size());
  for (size_t i = 0; i < s.size(); ++i)
    if (s[i] == '1')
      bv.set(i, true);

  // Returns the text.
  return is;
}

#endif // BITVECTOC

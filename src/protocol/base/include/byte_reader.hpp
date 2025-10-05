#pragma once

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define be64toh(x) OSSwapBigToHostInt64(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#else
#include <endian.h>
#endif

#include <cstdint>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <type_traits>
#include <vector>

template <typename Derived> class ByteReader {
protected:
  std::ifstream &file;

  ByteReader(std::ifstream &file) : file(file) {}

  template <typename T> Derived &readRaw(T &value) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "Type must be trivially copyable");
    file.read(reinterpret_cast<char *>(&value), sizeof(T));
    return *static_cast<Derived *>(this);
  }

  Derived &readBytes(uint8_t *buffer, size_t length) {
    file.read(reinterpret_cast<char *>(buffer), length);
    return *static_cast<Derived *>(this);
  }

  Derived &readBytes(std::vector<uint8_t> &buffer, size_t length) {
    buffer.resize(length);
    file.read(reinterpret_cast<char *>(buffer.data()), length);
    return *static_cast<Derived *>(this);
  }

  Derived &skipBytes(size_t count) {
    file.seekg(count, std::ios::cur);
    return *static_cast<Derived *>(this);
  }

  Derived &readUint16(uint16_t &value) {
    readRaw(value);
    value = ntohs(value);
    return *static_cast<Derived *>(this);
  }

  Derived &readInt16(int16_t &value) {
    readRaw(value);
    value = ntohs(value);
    return *static_cast<Derived *>(this);
  }

  Derived &readUint32(uint32_t &value) {
    readRaw(value);
    value = ntohl(value);
    return *static_cast<Derived *>(this);
  }

  Derived &readInt32(int32_t &value) {
    readRaw(value);
    value = ntohl(value);
    return *static_cast<Derived *>(this);
  }

  Derived &readUint64(uint64_t &value) {
    readRaw(value);
    value = be64toh(value);
    return *static_cast<Derived *>(this);
  }

  Derived &readInt64(int64_t &value) {
    readRaw(value);
    value = be64toh(value);
    return *static_cast<Derived *>(this);
  }

  Derived &readVarint(int32_t &value) {
    value = 0;
    int shift = 0;
    uint8_t byte;
    do {
      readRaw(byte);
      value |= (byte & 0x7f) << shift;
      shift += 7;
    } while (byte & 0x80);
    return *static_cast<Derived *>(this);
  }

  Derived &readZigZagVarint(int32_t &value) {
    int32_t n;
    readVarint(n);
    value = (n >> 1) ^ -(n & 1);
    return *static_cast<Derived *>(this);
  }

  Derived &readCompactString(std::string &value) {
    int16_t length;
    readInt16(length);
    std::vector<char> buffer(length);
    file.read(buffer.data(), length);
    value.assign(buffer.data(), length);
    return *static_cast<Derived *>(this);
  }

  template <typename T> Derived &readCompactArray(std::vector<T> &values) {
    uint8_t count;
    readRaw(count);
    count--; // Compact format
    values.clear();
    values.reserve(count);
    for (int i = 0; i < count; i++) {
      T value;
      readRaw(value);
      values.push_back(value);
    }
    return *static_cast<Derived *>(this);
  }
};

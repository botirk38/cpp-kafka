#pragma once

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define STORAGE_be64toh(x) OSSwapBigToHostInt64(x)
#else
#include <endian.h>
#define STORAGE_be64toh(x) be64toh(x)
#endif

#include <cstdint>
#include <fstream>
#include <netinet/in.h>
#include <type_traits>
#include <vector>

namespace storage::io {

class BinaryCursor {
public:
  explicit BinaryCursor(std::ifstream &file) : file_(file) {}

  template <typename T> BinaryCursor &readRaw(T &value) {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    file_.read(reinterpret_cast<char *>(&value), sizeof(T));
    return *this;
  }

  BinaryCursor &readBytes(uint8_t *buffer, size_t length) {
    file_.read(reinterpret_cast<char *>(buffer), length);
    return *this;
  }

  BinaryCursor &readBytes(std::vector<uint8_t> &buffer, size_t length) {
    buffer.resize(length);
    file_.read(reinterpret_cast<char *>(buffer.data()), length);
    return *this;
  }

  BinaryCursor &skipBytes(size_t count) {
    file_.seekg(static_cast<std::streamoff>(count), std::ios::cur);
    return *this;
  }

  BinaryCursor &readInt16(int16_t &value) {
    readRaw(value);
    value = ntohs(value);
    return *this;
  }

  BinaryCursor &readInt32(int32_t &value) {
    readRaw(value);
    value = ntohl(value);
    return *this;
  }

  BinaryCursor &readUint32(uint32_t &value) {
    readRaw(value);
    value = ntohl(value);
    return *this;
  }

  BinaryCursor &readInt64(int64_t &value) {
    readRaw(value);
    value = static_cast<int64_t>(STORAGE_be64toh(static_cast<uint64_t>(value)));
    return *this;
  }

  BinaryCursor &readVarint(int32_t &value) {
    value = 0;
    int shift = 0;
    uint8_t byte;
    do {
      readRaw(byte);
      value |= static_cast<int32_t>(byte & 0x7f) << shift;
      shift += 7;
    } while (byte & 0x80);
    return *this;
  }

  BinaryCursor &readZigZagVarint(int32_t &value) {
    int32_t n;
    readVarint(n);
    value = (n >> 1) ^ -(n & 1);
    return *this;
  }

  std::streampos tell() const { return file_.tellg(); }
  void seek(std::streampos pos) { file_.seekg(pos); }
  bool good() const { return file_.good(); }
  bool eof() const { return file_.eof(); }
  int peek() const { return file_.peek(); }

private:
  std::ifstream &file_;
};

} // namespace storage::io

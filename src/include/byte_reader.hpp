#pragma once

#include <cstdint>
#include <endian.h>
#include <fstream>
#include <netinet/in.h>
#include <type_traits>
#include <vector>

template <typename Derived> class ByteReader {
private:
  enum class IntegerSize { U16 = 2, U32 = 4, U64 = 8 };

protected:
  std::ifstream &file;

  ByteReader(std::ifstream &file) : file(file) {}

  template <typename T> Derived &readRaw(T &value) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "Type must be trivially copyable");
    file.read(reinterpret_cast<char *>(&value), sizeof(T));
    return *static_cast<Derived *>(this);
  }

  Derived &readBytes(std::vector<uint8_t> &buffer, size_t length) {
    buffer.resize(length);
    file.read(reinterpret_cast<char *>(buffer.data()), length);
    return *static_cast<Derived *>(this);
  }

  template <typename T> Derived &readNetworkOrder(T &value) {
    readRaw(value);
    constexpr auto size = sizeof(T);

    switch (static_cast<IntegerSize>(size)) {
    case IntegerSize::U16:
      value = ntohs(value);

    case IntegerSize::U32:
      value = ntohl(value);

    case IntegerSize::U64:
      value = be64toh(value);
    }
    return *static_cast<Derived *>(this);
  }

  Derived &readCompactString(std::string &value) {
    int16_t length;
    readNetworkOrder(length);
    std::vector<char> buffer(length);
    file.read(buffer.data(), length);
    value.assign(buffer.data(), length);
    return *static_cast<Derived *>(this);
  }
};

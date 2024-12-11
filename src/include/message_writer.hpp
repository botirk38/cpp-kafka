#pragma once
#include <arpa/inet.h>
#include <cstring>

template <typename Derived> class MessageWriter {
protected:
  char *buffer;
  int offset;

public:
  MessageWriter(char *buf) : buffer(buf), offset(0) {}

  Derived &writeInt32(int32_t value) {
    value = htonl(value);
    memcpy(buffer + offset, &value, sizeof(value));
    offset += sizeof(value);
    return *static_cast<Derived *>(this);
  }

  Derived &writeInt16(int16_t value) {
    value = htons(value);
    memcpy(buffer + offset, &value, sizeof(value));
    offset += sizeof(value);
    return *static_cast<Derived *>(this);
  }

  Derived &writeUInt8(uint8_t value) {
    memcpy(buffer + offset, &value, sizeof(value));
    offset += sizeof(value);
    return *static_cast<Derived *>(this);
  }

  Derived &writeInt8(int8_t value) {
    memcpy(buffer + offset, &value, sizeof(value));
    offset += sizeof(value);
    return *static_cast<Derived *>(this);
  }

  Derived &skipBytes(int count) {
    offset += count;
    return *static_cast<Derived *>(this);
  }

  void updateMessageSize() {
    int32_t message_size = htonl(offset - 4);
    memcpy(buffer, &message_size, 4);
  }

  int getOffset() const { return offset; }
};

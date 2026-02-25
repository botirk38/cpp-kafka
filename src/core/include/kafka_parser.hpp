#pragma once

#include "../../protocol/base/include/api_keys.hpp"
#include "../../protocol/base/include/kafka_request.hpp"
#include "../../protocol/requests/include/api_version_request.hpp"
#include "../../protocol/requests/include/describe_topic_partitions_request.hpp"
#include "../../protocol/requests/include/fetch_request.hpp"
#include "../../protocol/requests/include/kafka_request_variant.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

class ParseError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class Parser {
public:
  static KafkaRequestVariant parse(const uint8_t *data, size_t length);

private:
  class Buffer {
  public:
    explicit Buffer(const uint8_t *data, size_t length);

    // Network byte order read operations
    int16_t readInt16();
    int32_t readInt32();
    int8_t readInt8();
    uint8_t readUInt8();
    int64_t readInt64();
    uint128_t readUint128();
    void readBytes(uint8_t *dest, size_t length);

    // String operations
    std::string readString();        // Regular string
    std::string readCompactString(); // Kafka compact string

    // Buffer operations
    void skip(size_t n);
    const uint8_t *current() const;
    size_t remaining() const;
    void advance(size_t n);

  private:
    template <typename T> T readRaw() {
      if (remaining() < sizeof(T)) {
        throw ParseError("Buffer underflow");
      }
      T value;
      std::memcpy(&value, current(), sizeof(T));
      advance(sizeof(T));
      return value;
    }

    const uint8_t *data;
    size_t length;
    size_t offset{0};
  };

  static RequestHeader parseHeader(Buffer &buffer);
  static ApiVersionRequest parseApiVersion(Buffer &buffer, RequestHeader header);
  static DescribeTopicsRequest parseDescribeTopics(Buffer &buffer, RequestHeader header);
  static FetchRequest parseFetch(Buffer &buffer, RequestHeader header);
};

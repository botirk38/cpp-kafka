#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

class ParseError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct RequestHeader {
  int16_t api_key;
  int16_t api_version;
  int32_t correlation_id;
  std::string client_id;
};

class KafkaRequest {
public:
  virtual ~KafkaRequest() = default;
  RequestHeader header;
};

class ApiVersionRequest : public KafkaRequest {};

class DescribeTopicsRequest : public KafkaRequest {
public:
  std::vector<std::string> topic_names;
  int32_t response_partition_limit;
  struct Cursor {
    std::string topic_name;
    int32_t partition_index;
  };
  std::optional<Cursor> cursor;
};

class Parser {
public:
  static std::unique_ptr<KafkaRequest> parse(const uint8_t *data,
                                             size_t length);

private:
  class Buffer {
  public:
    explicit Buffer(const uint8_t *data, size_t length);

    // Network byte order read operations
    int16_t readInt16();
    int32_t readInt32();
    int8_t readInt8();
    uint8_t readUInt8();

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
  static std::unique_ptr<ApiVersionRequest>
  parseApiVersion(Buffer &buffer, RequestHeader header);
  static std::unique_ptr<DescribeTopicsRequest>
  parseDescribeTopics(Buffer &buffer, RequestHeader header);
};

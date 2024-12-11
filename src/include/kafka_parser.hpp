#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
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

class ApiVersionRequest : public KafkaRequest {
public:
};

class DescribeTopicsRequest : public KafkaRequest {
public:
  std::vector<std::string> topic_names;
};

class Parser {
public:
  static std::unique_ptr<KafkaRequest> parse(const uint8_t *data,
                                             size_t length);

private:
  class Buffer {
  public:
    explicit Buffer(const uint8_t *data, size_t length);

    template <typename T> T read() {
      if (remaining() < sizeof(T)) {
        throw ParseError("Buffer underflow");
      }
      T value;
      std::memcpy(&value, current(), sizeof(T));
      advance(sizeof(T));
      return value;
    }

    std::string readString();
    void skip(size_t n);
    const uint8_t *current() const { return data + offset; }
    size_t remaining() const { return length - offset; }
    void advance(size_t n);

  private:
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

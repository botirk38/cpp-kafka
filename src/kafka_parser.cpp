#include "include/kafka_parser.hpp"
#include "include/api_version_response.hpp"
#include "include/describe_topics_partitions_response.hpp"
#include <arpa/inet.h>
#include <cstring>

using ApiVersion = ApiVersionResponse::ApiVersions;
using DescribeTopics = DescribeTopicPartitionsResponse::DescribeTopicPartitions;

Parser::Buffer::Buffer(const uint8_t *data, size_t length)
    : data(data), length(length) {}

void Parser::Buffer::advance(size_t n) {
  if (n > remaining()) {
    throw ParseError("Buffer overflow");
  }
  offset += n;
}

std::string Parser::Buffer::readString() {
  auto len = ntohs(read<int16_t>());
  if (len < 0 || static_cast<size_t>(len) > remaining()) {
    throw ParseError("Invalid string length");
  }
  std::string result(reinterpret_cast<const char *>(current()), len);
  advance(len);
  return result;
}

void Parser::Buffer::skip(size_t n) { advance(n); }

std::unique_ptr<KafkaRequest> Parser::parse(const uint8_t *data,
                                            size_t length) {
  Buffer buffer(data, length);

  if (length < 12) {
    throw ParseError("Message too short");
  }

  auto header = parseHeader(buffer);

  switch (header.api_key) {
  case ApiVersion::KEY:
    return parseApiVersion(buffer, std::move(header));
  case DescribeTopics::KEY:
    return parseDescribeTopics(buffer, std::move(header));
  default:
    throw ParseError("Unknown API key: " + std::to_string(header.api_key));
  }
}

RequestHeader Parser::parseHeader(Buffer &buffer) {
  buffer.skip(4); // Skip size

  RequestHeader header;
  header.api_key = ntohs(buffer.read<int16_t>());
  header.api_version = ntohs(buffer.read<int16_t>());
  header.correlation_id = ntohl(buffer.read<int32_t>());
  header.client_id = buffer.readString();

  return header;
}

std::unique_ptr<ApiVersionRequest>
Parser::parseApiVersion(Buffer &buffer, RequestHeader header) {
  auto request = std::make_unique<ApiVersionRequest>();
  request->header = std::move(header);
  return request;
}

std::unique_ptr<DescribeTopicsRequest>
Parser::parseDescribeTopics(Buffer &buffer, RequestHeader header) {
  auto request = std::make_unique<DescribeTopicsRequest>();
  request->header = std::move(header);

  buffer.skip(1); // TAG_BUFFER

  auto array_length = buffer.read<int8_t>() - 1;
  for (int i = 0; i < array_length; i++) {
    auto topic_length = buffer.read<uint8_t>() - 1;
    std::string topic_name(reinterpret_cast<const char *>(buffer.current()),
                           topic_length);
    buffer.advance(topic_length);
    request->topic_names.push_back(std::move(topic_name));
  }

  return request;
}

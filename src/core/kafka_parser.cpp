#include "include/kafka_parser.hpp"
#include <arpa/inet.h>

namespace KP = KafkaProtocol;

Parser::Buffer::Buffer(const uint8_t *data, size_t length) : data(data), length(length) {}

void Parser::Buffer::advance(size_t n) {
  if (n > remaining()) {
    throw ParseError("Buffer overflow");
  }
  offset += n;
}

const uint8_t *Parser::Buffer::current() const { return data + offset; }

size_t Parser::Buffer::remaining() const { return length - offset; }

void Parser::Buffer::skip(size_t n) { advance(n); }

int16_t Parser::Buffer::readInt16() { return ntohs(readRaw<int16_t>()); }

int32_t Parser::Buffer::readInt32() { return ntohl(readRaw<int32_t>()); }

int8_t Parser::Buffer::readInt8() { return readRaw<int8_t>(); }

uint8_t Parser::Buffer::readUInt8() { return readRaw<uint8_t>(); }

int64_t Parser::Buffer::readInt64() { return readRaw<int64_t>(); }

uint128_t Parser::Buffer::readUint128() {
  uint128_t result = 0;
  for (int i = 0; i < 16; i++) {
    result = (result << 8) | readUInt8();
  }
  return result;
}

void Parser::Buffer::readBytes(uint8_t *dest, size_t length) {
  if (remaining() < length) {
    throw ParseError("Buffer underflow");
  }
  std::memcpy(dest, current(), length);
  advance(length);
}

std::string Parser::Buffer::readString() {
  auto len = readInt16();
  if (len < 0 || static_cast<size_t>(len) > remaining()) {
    throw ParseError("Invalid string length");
  }
  std::string result(reinterpret_cast<const char *>(current()), len);
  advance(len);
  return result;
}

std::string Parser::Buffer::readCompactString() {
  auto len = readUInt8();
  if (len == 0) {
    return "";
  }
  len--;

  if (len > remaining()) {
    throw ParseError("Invalid compact string length");
  }

  std::string result(reinterpret_cast<const char *>(current()), len);
  advance(len);
  return result;
}

KafkaRequestVariant Parser::parse(const uint8_t *data, size_t length) {
  Buffer buffer(data, length);
  if (length < 12) {
    throw ParseError("Message too short");
  }

  auto header = parseHeader(buffer);
  switch (header.api_key) {
  case KP::API_VERSIONS:
    return parseApiVersion(buffer, std::move(header));
  case KP::DESCRIBE_TOPIC_PARTITIONS:
    return parseDescribeTopics(buffer, std::move(header));
  case KP::FETCH:
    return parseFetch(buffer, std::move(header));
  default:
    throw ParseError("Unknown API key: " + std::to_string(header.api_key));
  }
}

RequestHeader Parser::parseHeader(Buffer &buffer) {
  buffer.skip(4); // Skip size

  RequestHeader header;
  header.api_key = buffer.readInt16();
  header.api_version = buffer.readInt16();
  header.correlation_id = buffer.readInt32();
  header.client_id = buffer.readString();
  return header;
}

ApiVersionRequest Parser::parseApiVersion(Buffer &buffer, RequestHeader header) {
  ApiVersionRequest request;
  request.header = std::move(header);
  return request;
}

DescribeTopicsRequest Parser::parseDescribeTopics(Buffer &buffer, RequestHeader header) {
  DescribeTopicsRequest request;
  request.header = std::move(header);

  buffer.skip(1); // TAG_BUFFER
  int topics_length = buffer.readInt8() - 1;
  if (topics_length < 0 || static_cast<size_t>(topics_length) > buffer.remaining()) {
    throw ParseError("Invalid topics array length");
  }

  for (int i = 0; i < topics_length; i++) {
    auto topic_name = buffer.readCompactString();
    buffer.skip(1); // TAG_BUFFER for topic
    request.topic_names.push_back(std::move(topic_name));
  }

  request.response_partition_limit = buffer.readInt32();

  auto cursor_tag = buffer.readUInt8();
  if (cursor_tag != 0xFF) {
    DescribeTopicsRequest::Cursor cursor;
    cursor.topic_name = buffer.readCompactString();
    cursor.partition_index = buffer.readInt32();
    request.cursor = cursor;
  }

  buffer.skip(1); // Final TAG_BUFFER
  return request;
}

FetchRequest Parser::parseFetch(Buffer &buffer, RequestHeader header) {
  FetchRequest request;
  request.header = header;
  buffer.skip(1);

  request.max_wait_ms = buffer.readInt32();
  request.min_bytes = buffer.readInt32();
  request.max_bytes = buffer.readInt32();
  request.isolation_level = buffer.readInt8();
  request.session_id = buffer.readInt32();
  request.session_epoch = buffer.readInt32();

  int topics_length = buffer.readInt8() - 1;
  if (topics_length < 0 || static_cast<size_t>(topics_length) > buffer.remaining()) {
    throw ParseError("Invalid fetch topics array length");
  }

  for (int i = 0; i < topics_length; i++) {
    FetchTopic topic;
    topic.topic_id = buffer.readUint128();

    int partitions_length = buffer.readInt8() - 1;
    if (partitions_length < 0 || static_cast<size_t>(partitions_length) > buffer.remaining()) {
      throw ParseError("Invalid fetch partitions array length");
    }

    for (int j = 0; j < partitions_length; j++) {
      FetchPartition partition;
      partition.partition = buffer.readInt32();
      partition.current_leader_epoch = buffer.readInt32();
      partition.fetch_offset = buffer.readInt64();
      partition.last_fetched_epoch = buffer.readInt32();
      partition.log_start_offset = buffer.readInt64();
      partition.partition_max_bytes = buffer.readInt32();
      topic.partitions.push_back(partition);
    }

    request.topics.push_back(topic);
  }

  int8_t forgotten_topics_length = buffer.readInt8() - 1;
  if (forgotten_topics_length < 0 ||
      static_cast<size_t>(forgotten_topics_length) > buffer.remaining()) {
    throw ParseError("Invalid forgotten topics array length");
  }

  for (int8_t i = 0; i < forgotten_topics_length; i++) {
    ForgottenTopic topic;
    topic.topic_id = buffer.readUint128();

    int partitions_length = buffer.readInt8() - 1;

    for (int j = 0; j < partitions_length; j++) {
      topic.partitions.push_back(buffer.readInt32());
    }
    request.forgotten_topics_data.push_back(topic);
  }

  request.rack_id = buffer.readCompactString();

  return request;
}

#include "include/kafka_parser.hpp"
#include "include/api_version_response.hpp"
#include "include/describe_topics_partitions_response.hpp"
#include "include/fetch_response.hpp"
#include <arpa/inet.h>
#include <iostream>

using ApiVersion = ApiVersionResponse::ApiVersions;
using DescribeTopics = DescribeTopicPartitionsResponse::DescribeTopicPartitions;
using Fetch = FetchResponse::Fetch;

Parser::Buffer::Buffer(const uint8_t *data, size_t length)
    : data(data), length(length) {}

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
  auto len = readUInt8() - 1;
  std::string result(reinterpret_cast<const char *>(current()), len);
  advance(len);
  return result;
}

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
  case Fetch::KEY:
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

  std::cout << "Parsing DescribeTopics request" << std::endl;

  buffer.skip(1); // TAG_BUFFER
  auto topics_length = buffer.readInt8() - 1;
  std::cout << "Topics array length: " << static_cast<int>(topics_length)
            << std::endl;

  for (int i = 0; i < topics_length; i++) {
    auto topic_name = buffer.readCompactString();
    buffer.skip(1); // TAG_BUFFER for topic
    request->topic_names.push_back(std::move(topic_name));
  }

  request->response_partition_limit = buffer.readInt32();

  auto cursor_tag = buffer.readUInt8();
  if (cursor_tag != 0xFF) {
    DescribeTopicsRequest::Cursor cursor;
    cursor.topic_name = buffer.readCompactString();
    cursor.partition_index = buffer.readInt32();
    request->cursor = cursor;
  }

  buffer.skip(1); // Final TAG_BUFFER
  return request;
}

std::unique_ptr<FetchRequest> Parser::parseFetch(Buffer &buffer,
                                                 RequestHeader header) {
  auto request = std::make_unique<FetchRequest>();
  request->header = header;

  request->max_wait_ms = buffer.readInt32();
  request->min_bytes = buffer.readInt32();
  request->max_bytes = buffer.readInt32();
  request->isolation_level = buffer.readInt8();
  request->session_id = buffer.readInt32();
  request->session_epoch = buffer.readInt32();

  // Read topics array
  int topics_length = buffer.readInt8() - 1; // Compact array
  for (int i = 0; i < topics_length; i++) {
    FetchTopic topic;
    buffer.readBytes(topic.topic_id.data(), 16); // Read UUID

    int partitions_length = buffer.readInt8() - 1; // Compact array
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
    buffer.readInt8(); // Tag buffer
    request->topics.push_back(topic);
  }

  // Read forgotten topics
  int forgotten_topics_length = buffer.readInt8() - 1; // Compact array
  for (int i = 0; i < forgotten_topics_length; i++) {
    ForgottenTopic topic;
    buffer.readBytes(topic.topic_id.data(), 16); // Read UUID

    int partitions_length = buffer.readInt8() - 1; // Compact array
    for (int j = 0; j < partitions_length; j++) {
      topic.partitions.push_back(buffer.readInt32());
    }
    request->forgotten_topics_data.push_back(topic);
  }

  request->rack_id = buffer.readCompactString();
  buffer.readInt8(); // Tag buffer

  return request;
}

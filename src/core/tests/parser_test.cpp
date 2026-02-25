#include "../../protocol/base/include/api_keys.hpp"
#include "../../protocol/requests/include/describe_topic_partitions_request.hpp"
#include "../../protocol/requests/include/fetch_request.hpp"
#include "../../protocol/requests/include/kafka_request_variant.hpp"
#include "../include/kafka_parser.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <gtest/gtest.h>
#include <vector>

namespace KP = KafkaProtocol;

static void writeInt32(uint8_t *p, int32_t v) {
  int32_t n = htonl(v);
  memcpy(p, &n, 4);
}
static void writeInt16(uint8_t *p, int16_t v) {
  int16_t n = htons(v);
  memcpy(p, &n, 2);
}
static void writeInt64(uint8_t *p, int64_t v) {
  uint64_t u = static_cast<uint64_t>(v);
  for (int i = 7; i >= 0; i--) {
    p[i] = static_cast<uint8_t>(u & 0xFF);
    u >>= 8;
  }
}

TEST(ParserTest, MessageTooShort) {
  std::vector<uint8_t> buf(11, 0);
  EXPECT_THROW(Parser::parse(buf.data(), buf.size()), ParseError);
}

TEST(ParserTest, UnknownApiKey) {
  std::vector<uint8_t> buf(20, 0);
  writeInt32(buf.data() + 0, 6);
  writeInt16(buf.data() + 4, 99);
  writeInt16(buf.data() + 6, 0);
  writeInt32(buf.data() + 8, 1);
  writeInt16(buf.data() + 12, 0);
  EXPECT_THROW(Parser::parse(buf.data(), buf.size()), ParseError);
}

TEST(ParserTest, ApiVersionsRequest) {
  std::vector<uint8_t> buf(20, 0);
  writeInt32(buf.data() + 0, 6);
  writeInt16(buf.data() + 4, KP::API_VERSIONS);
  writeInt16(buf.data() + 6, 0);
  writeInt32(buf.data() + 8, 42);
  writeInt16(buf.data() + 12, 0);

  auto req = Parser::parse(buf.data(), buf.size());
  ASSERT_TRUE(std::holds_alternative<ApiVersionRequest>(req));
  const auto &r = std::get<ApiVersionRequest>(req);
  EXPECT_EQ(r.header.api_key, KP::API_VERSIONS);
  EXPECT_EQ(r.header.correlation_id, 42);
}

TEST(ParserTest, GetApiKey) {
  std::vector<uint8_t> buf(20, 0);
  writeInt32(buf.data() + 0, 6);
  writeInt16(buf.data() + 4, KP::API_VERSIONS);
  writeInt16(buf.data() + 6, 0);
  writeInt32(buf.data() + 8, 1);
  writeInt16(buf.data() + 12, 0);

  auto req = Parser::parse(buf.data(), buf.size());
  EXPECT_EQ(getApiKey(req), KP::API_VERSIONS);
}

TEST(ParserTest, DescribeTopicsRequest) {
  // Header: size(4), api_key(2), api_version(2), correlation_id(4), client_id(2)
  // Body: TAG(1), topics_len(1)=2, topic "a"(2), TAG(1), limit(4), cursor_tag(1)=0xFF, TAG(1)
  std::vector<uint8_t> buf(30, 0);
  size_t off = 0;
  writeInt32(buf.data() + off, 21);
  off += 4;
  writeInt16(buf.data() + off, KP::DESCRIBE_TOPIC_PARTITIONS);
  off += 2;
  writeInt16(buf.data() + off, 4);
  off += 2;
  writeInt32(buf.data() + off, 99);
  off += 4;
  writeInt16(buf.data() + off, 0);
  off += 2;
  buf[off++] = 0; // TAG_BUFFER
  buf[off++] = 2; // 1 topic (compact: count+1)
  buf[off++] = 2; // "a" (compact: len+1)
  buf[off++] = 'a';
  buf[off++] = 0; // TAG_BUFFER
  writeInt32(buf.data() + off, 100);
  off += 4;
  buf[off++] = 0xFF; // no cursor
  buf[off++] = 0;    // TAG_BUFFER

  auto req = Parser::parse(buf.data(), buf.size());
  ASSERT_TRUE(std::holds_alternative<DescribeTopicsRequest>(req));
  const auto &r = std::get<DescribeTopicsRequest>(req);
  EXPECT_EQ(r.header.api_key, KP::DESCRIBE_TOPIC_PARTITIONS);
  EXPECT_EQ(r.header.correlation_id, 99);
  ASSERT_EQ(r.topic_names.size(), 1u);
  EXPECT_EQ(r.topic_names[0], "a");
  EXPECT_EQ(r.response_partition_limit, 100);
  EXPECT_FALSE(r.cursor.has_value());
}

TEST(ParserTest, FetchRequest) {
  // Header + body with 1 topic, 1 partition, 0 forgotten, empty rack_id
  // Kafka Fetch v16: TAG_BUFFER, max_wait_ms, min_bytes, max_bytes, isolation_level,
  // session_id, session_epoch, topics[], forgotten_topics[], rack_id
  std::vector<uint8_t> buf;
  auto append = [&buf](const uint8_t *p, size_t n) { buf.insert(buf.end(), p, p + n); };
  uint8_t tmp[8];
  writeInt32(tmp, 83);
  append(tmp, 4);
  writeInt16(tmp, KP::FETCH);
  append(tmp, 2);
  writeInt16(tmp, 16);
  append(tmp, 2);
  writeInt32(tmp, 7);
  append(tmp, 4);
  writeInt16(tmp, 0);
  append(tmp, 2);
  buf.push_back(0); // TAG_BUFFER
  writeInt32(tmp, 500);
  append(tmp, 4);
  writeInt32(tmp, 1);
  append(tmp, 4);
  writeInt32(tmp, 1024);
  append(tmp, 4);
  buf.push_back(0); // isolation_level
  writeInt32(tmp, 0);
  append(tmp, 4);
  writeInt32(tmp, 0);
  append(tmp, 4);
  buf.push_back(2); // 1 topic (compact: count+1)
  for (int i = 0; i < 16; i++)
    buf.push_back((i == 15) ? 1 : 0); // topic_id
  buf.push_back(2);                   // 1 partition
  writeInt32(tmp, 0);
  append(tmp, 4);
  writeInt32(tmp, 0);
  append(tmp, 4);
  writeInt64(tmp, 0);
  append(tmp, 8);
  writeInt32(tmp, 0);
  append(tmp, 4);
  writeInt64(tmp, 0);
  append(tmp, 8);
  writeInt32(tmp, 4096);
  append(tmp, 4);
  buf.push_back(1); // 0 forgotten topics
  buf.push_back(0); // empty rack_id

  auto req = Parser::parse(buf.data(), buf.size());
  ASSERT_TRUE(std::holds_alternative<FetchRequest>(req));
  const auto &r = std::get<FetchRequest>(req);
  EXPECT_EQ(r.header.api_key, KP::FETCH);
  EXPECT_EQ(r.header.correlation_id, 7);
  EXPECT_EQ(r.max_wait_ms, 500);
  EXPECT_EQ(r.min_bytes, 1);
  EXPECT_EQ(r.max_bytes, 1024);
  ASSERT_EQ(r.topics.size(), 1u);
  ASSERT_EQ(r.topics[0].partitions.size(), 1u);
  EXPECT_EQ(r.topics[0].partitions[0].partition, 0);
  EXPECT_EQ(r.topics[0].partitions[0].partition_max_bytes, 4096);
  EXPECT_TRUE(r.forgotten_topics_data.empty());
  EXPECT_TRUE(r.rack_id.empty());
}

#include "../include/metadata_decoder.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace KafkaMetadata;

TEST(MetadataDecoderTest, TopicRecordTooShort) {
  std::vector<uint8_t> data(2, 0);
  EXPECT_THROW(decodeTopicRecord(data), DecodeError);
}

TEST(MetadataDecoderTest, TopicRecordInvalidNameLength) {
  std::vector<uint8_t> data(20, 0);
  data[3] = 0;
  EXPECT_THROW(decodeTopicRecord(data), DecodeError);
}

TEST(MetadataDecoderTest, TopicRecordNameExtendsPastBuffer) {
  std::vector<uint8_t> data(10, 0);
  data[3] = 10;
  EXPECT_THROW(decodeTopicRecord(data), DecodeError);
}

TEST(MetadataDecoderTest, PartitionRecordTooShort) {
  std::vector<uint8_t> data(2, 0);
  EXPECT_THROW(decodePartitionRecord(data), DecodeError);
}

TEST(MetadataDecoderTest, TopicRecordValid) {
  std::vector<uint8_t> data(3 + 1 + 4 + 16, 0);
  data[3] = 5;
  data[4] = 't';
  data[5] = 'e';
  data[6] = 's';
  data[7] = 't';

  auto topic = decodeTopicRecord(data);
  EXPECT_EQ(topic.name, "test");
  EXPECT_EQ(static_cast<uint64_t>(topic.topic_id), 0u);
}

#pragma once

#include "kafka_metadata.hpp"
#include "message_writer.hpp"
#include <array>
#include <optional>
#include <string>

class DescribeTopicPartitionsResponse
    : public MessageWriter<DescribeTopicPartitionsResponse> {
public:
  DescribeTopicPartitionsResponse(char *buf) : MessageWriter(buf) {}

  DescribeTopicPartitionsResponse &writeHeader(int32_t correlation_id,
                                               int8_t topics_length);

  DescribeTopicPartitionsResponse &
  writeTopic(const std::string &topic_name,
             const std::optional<KafkaMetadata::TopicMetadata> &topic_metadata);
  DescribeTopicPartitionsResponse &complete();

  enum DescribeTopicPartitions {
    KEY = 75,
    ERROR_UNKNOWN_TOPIC = 3,
    TAG_BUFFER = 0
  };

private:
  DescribeTopicPartitionsResponse &writeTopicMetadata(
      const std::string &topic_name, const std::array<uint8_t, 16> &topic_id,
      const std::vector<KafkaMetadata::PartitionMetadata> &partition_metadata);

  DescribeTopicPartitionsResponse &writePartitionMetadata(int32_t partition_id);
  DescribeTopicPartitionsResponse &
  writeUnknownTopicError(const std::string &topic_name);
};

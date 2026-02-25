#pragma once

#include "../../../storage/include/storage_types.hpp"
#include "../../base/include/message_writer.hpp"
#include <optional>
#include <string>

class DescribeTopicPartitionsResponse : public MessageWriter<DescribeTopicPartitionsResponse> {
public:
  DescribeTopicPartitionsResponse(char *buf) : MessageWriter(buf) {}

  DescribeTopicPartitionsResponse &writeHeader(int32_t correlation_id, int8_t topics_length);

  DescribeTopicPartitionsResponse &writeTopic(const std::string &topic_name,
                                              const std::optional<storage::TopicInfo> &topic_info);
  DescribeTopicPartitionsResponse &complete();

  enum DescribeTopicPartitions { KEY = 75, ERROR_UNKNOWN_TOPIC = 3, TAG_BUFFER = 0 };

private:
  DescribeTopicPartitionsResponse &
  writeTopicMetadata(const std::string &topic_name, storage::TopicId topic_id,
                     const std::vector<storage::PartitionInfo> &partitions);

  DescribeTopicPartitionsResponse &writePartitionMetadata(int32_t partition_id);
  DescribeTopicPartitionsResponse &writeUnknownTopicError(const std::string &topic_name);
};

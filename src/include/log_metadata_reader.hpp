#pragma once

#include "kafka_metadata.hpp"
#include "record_batch_reader.hpp"
#include <optional>
#include <string>

using uint128_t = __uint128_t;

class KafkaLogMetadataReader {

public:
  using PartitionMetadata = KafkaMetadata::PartitionMetadata;
  using TopicMetadata = KafkaMetadata::TopicMetadata;

  explicit KafkaLogMetadataReader(const std::string &log_path);

  std::optional<TopicMetadata> findTopic(const std::string &topic_name,
                                         std::optional<int> partition_id = -1);

  std::optional<TopicMetadata> findTopicById(const uint128_t &topic_id);

  PartitionMetadata parsePartitionMetadata(const RecordReader::Record &record);

private:
  std::string log_path;

  TopicMetadata parseTopicMetadata(const RecordReader::Record &record);
  std::vector<RecordBatchReader::RecordBatch>
  readAllBatches(std::ifstream &file);
};

#pragma once 

#include "kafka_metadata.hpp"
#include "record_batch_reader.hpp"
#include <optional>
#include <string>

class KafkaLogMetadataReader {

public:
  using PartitionMetadata = KafkaMetadata::PartitionMetadata;
  using TopicMetadata = KafkaMetadata::TopicMetadata;

  explicit KafkaLogMetadataReader(const std::string &log_path);

  std::optional<TopicMetadata> findTopic(const std::string &topic_name,
                                         std::optional<int> partition_id = -1);

  PartitionMetadata parsePartitionMetadata(const RecordReader::Record &record);

private:
  std::string log_path;

  TopicMetadata parseTopicMetadata(const RecordReader::Record &record);
  std::vector<RecordBatchReader::RecordBatch>
  readAllBatches(std::ifstream &file);
};

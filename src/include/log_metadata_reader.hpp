#pragma once
#include "kafka_metadata.hpp"
#include "record_batch_reader.hpp"
#include <string>

class KafkaLogMetadataReader {
public:
  using ClusterMetadata = KafkaMetadata::ClusterMetadata;
  using TopicMetadata = KafkaMetadata::TopicMetadata;
  using PartitionMetadata = KafkaMetadata::PartitionMetadata;

  explicit KafkaLogMetadataReader(const std::string &base_path);

  ClusterMetadata loadClusterMetadata();
  std::vector<RecordBatchReader::RecordBatch>
  readPartitionLog(const std::string &topic_name, int32_t partition_id);

private:
  const std::string base_path;
  static constexpr uint8_t TOPIC_RECORD = 0x02;
  static constexpr uint8_t PARTITION_RECORD = 0x03;
  static constexpr char LOG_FILE[] = "00000000000000000000.log";
  std::vector<RecordBatchReader::RecordBatch>
  readAllBatches(std::ifstream &file);

  TopicMetadata parseTopicMetadata(const RecordReader::Record &record);
  PartitionMetadata parsePartitionMetadata(const RecordReader::Record &record);
  std::string getPartitionLogPath(const std::string &topic_name,
                                  int32_t partition_id);
};

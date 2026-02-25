#include "include/log_metadata_reader.hpp"
#include "include/metadata_decoder.hpp"

KafkaLogMetadataReader::KafkaLogMetadataReader(const std::string &base_path)
    : base_path(base_path) {}

std::vector<RecordBatchReader::RecordBatch>
KafkaLogMetadataReader::readAllBatches(std::ifstream &file) {
  std::vector<RecordBatchReader::RecordBatch> batches;

  while (file.good() && !file.eof()) {
    if (file.peek() == EOF) {
      break;
    }

    auto batch = RecordBatchReader(file).readHeader().readMetadata().readRecords().complete();

    batches.push_back(std::move(batch));
  }
  return batches;
}

KafkaMetadata::ClusterMetadata KafkaLogMetadataReader::loadClusterMetadata() {
  KafkaMetadata::ClusterMetadata metadata;
  std::string metadata_path = base_path + "/__cluster_metadata-0/" + LOG_FILE;

  std::ifstream file(metadata_path, std::ios::binary);
  if (!file.is_open()) {
    return metadata;
  }

  auto batches = readAllBatches(file);

  for (const auto &batch : batches) {

    for (const auto &record : batch.records) {
      if (record.value.size() < 2) {
        continue;
      }
      if (record.value[1] == TOPIC_RECORD) {
        auto topic = KafkaMetadata::decodeTopicRecord(record.value);
        metadata.topics_by_id[topic.topic_id] = topic;
      } else if (record.value[1] == PARTITION_RECORD) {
        auto partition = KafkaMetadata::decodePartitionRecord(record.value);
        auto &topic = metadata.topics_by_id[partition.topic_id];
        partition.record_batches = readPartitionLog(topic.name, partition.partition_id);
        topic.partitions.push_back(partition);
      }
    }
  }

  return metadata;
}

std::vector<RecordBatchReader::RecordBatch>
KafkaLogMetadataReader::readPartitionLog(const std::string &topic_name, int32_t partition_id) {
  std::string log_path = getPartitionLogPath(topic_name, partition_id);

  std::ifstream file(log_path, std::ios::binary);
  if (!file.is_open()) {
    return {};
  }

  auto batches = readAllBatches(file);

  return batches;
}

std::string KafkaLogMetadataReader::getPartitionLogPath(const std::string &topic_name,
                                                        int32_t partition_id) {
  return base_path + "/" + topic_name + "-" + std::to_string(partition_id) + "/" + LOG_FILE;
}

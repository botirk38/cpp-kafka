#include "include/log_metadata_reader.hpp"
#include <iostream>

KafkaLogMetadataReader::KafkaLogMetadataReader(const std::string &base_path)
    : base_path(base_path) {}

std::vector<RecordBatchReader::RecordBatch>
KafkaLogMetadataReader::readAllBatches(std::ifstream &file) {
  std::vector<RecordBatchReader::RecordBatch> batches;

  while (file.good() && !file.eof()) {
    if (file.peek() == EOF) {
      break;
    }

    auto batch = RecordBatchReader(file)
                     .readHeader()
                     .readMetadata()
                     .readRecords()
                     .complete();

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
      if (record.value[1] == TOPIC_RECORD) {
        auto topic = parseTopicMetadata(record);
        metadata.topics_by_id[topic.topic_id] = topic;
      } else if (record.value[1] == PARTITION_RECORD) {
        auto partition = parsePartitionMetadata(record);
        auto &topic = metadata.topics_by_id[partition.topic_id];


        partition.record_batches =
            readPartitionLog(topic.name, partition.partition_id);
        topic.partitions.push_back(partition);

      }
    }
  }

  return metadata;
}

KafkaMetadata::TopicMetadata
KafkaLogMetadataReader::parseTopicMetadata(const RecordReader::Record &record) {
  KafkaMetadata::TopicMetadata metadata;
  const uint8_t *data = record.value.data();
  size_t pos = 3; // Skip frame version, type, version

  uint8_t name_length = data[pos] - 1;
  pos++;

  metadata.name =
      std::string(reinterpret_cast<const char *>(data + pos), name_length);
  pos += name_length;

  metadata.topic_id = 0;
  for (int i = 0; i < 16; i++) {
    metadata.topic_id = (metadata.topic_id << 8) | data[pos + i];
  }

  return metadata;
}

KafkaMetadata::PartitionMetadata KafkaLogMetadataReader::parsePartitionMetadata(
    const RecordReader::Record &record) {
  KafkaMetadata::PartitionMetadata metadata;
  const uint8_t *data = record.value.data();
  size_t pos = 3; // Skip frame version, type, version

  metadata.partition_id =
      static_cast<int32_t>((data[pos] << 24) | (data[pos + 1] << 16) |
                           (data[pos + 2] << 8) | data[pos + 3]);
  pos += 4;

  metadata.topic_id = 0;
  for (int i = 0; i < 16; i++) {
    metadata.topic_id = (metadata.topic_id << 8) | data[pos + i];
  }
  pos += 16;

  uint8_t replica_count = data[pos++] - 1;
  for (int i = 0; i < replica_count; i++) {
    int32_t replica_id =
        static_cast<int32_t>((data[pos] << 24) | (data[pos + 1] << 16) |
                             (data[pos + 2] << 8) | data[pos + 3]);
    metadata.replicas.push_back(replica_id);
    pos += 4;
  }

  uint8_t isr_count = data[pos++] - 1;
  for (int i = 0; i < isr_count; i++) {
    int32_t isr_id =
        static_cast<int32_t>((data[pos] << 24) | (data[pos + 1] << 16) |
                             (data[pos + 2] << 8) | data[pos + 3]);
    metadata.isr.push_back(isr_id);
    pos += 4;
  }

  pos += 2; // Skip removing and adding replicas arrays

  metadata.leader_id =
      static_cast<int32_t>((data[pos] << 24) | (data[pos + 1] << 16) |
                           (data[pos + 2] << 8) | data[pos + 3]);
  pos += 4;

  metadata.leader_epoch =
      static_cast<int32_t>((data[pos] << 24) | (data[pos + 1] << 16) |
                           (data[pos + 2] << 8) | data[pos + 3]);
  pos += 4;

  metadata.partition_epoch =
      static_cast<int32_t>((data[pos] << 24) | (data[pos + 1] << 16) |
                           (data[pos + 2] << 8) | data[pos + 3]);

  return metadata;
}

std::vector<RecordBatchReader::RecordBatch>
KafkaLogMetadataReader::readPartitionLog(const std::string &topic_name,
                                         int32_t partition_id) {
  std::string log_path = getPartitionLogPath(topic_name, partition_id);

  std::ifstream file(log_path, std::ios::binary);
  if (!file.is_open()) {
    return {};
  }

  auto batches = readAllBatches(file);

  return batches;
}

std::string
KafkaLogMetadataReader::getPartitionLogPath(const std::string &topic_name,
                                            int32_t partition_id) {
  return base_path + "/" + topic_name + "-" + std::to_string(partition_id) +
         "/" + LOG_FILE;
}

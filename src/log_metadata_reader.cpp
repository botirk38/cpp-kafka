#include "include/log_metadata_reader.hpp"
#include <iostream>
#include <sstream>

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

  std::cout << "Loading cluster metadata from: " << metadata_path << std::endl;

  std::ifstream file(metadata_path, std::ios::binary);
  if (!file.is_open()) {
    std::cout << "Failed to open metadata file" << std::endl;
    return metadata;
  }

  std::cout << "Reading metadata record batches" << std::endl;
  auto batches = readAllBatches(file);
  std::cout << "Found " << batches.size() << " record batches" << std::endl;

  for (const auto &batch : batches) {
    std::cout << "Processing batch with " << batch.records.size() << " records"
              << std::endl;

    for (const auto &record : batch.records) {
      if (record.value[1] == TOPIC_RECORD) {
        auto topic = parseTopicMetadata(record);
        std::stringstream ss;
        ss << std::hex << "0x" << static_cast<uint64_t>(topic.topic_id >> 64)
           << static_cast<uint64_t>(topic.topic_id);
        std::cout << "Found topic record: " << topic.name
                  << " (ID: " << ss.str() << ")" << std::endl;
        metadata.topics_by_id[topic.topic_id] = topic;
      } else if (record.value[1] == PARTITION_RECORD) {
        auto partition = parsePartitionMetadata(record);
        auto &topic = metadata.topics_by_id[partition.topic_id];

        std::cout << "Found partition record for topic " << topic.name
                  << ", partition " << partition.partition_id << std::endl;

        std::cout << "Loading partition log for " << topic.name << "-"
                  << partition.partition_id << std::endl;

        partition.record_batches =
            readPartitionLog(topic.name, partition.partition_id);
        std::cout << "Loaded " << partition.record_batches.size()
                  << " record batches for partition" << std::endl;

        topic.partitions.push_back(partition);

        std::cout << "Added to map" << std::endl;
      }
    }
  }

  std::cout << "Completed loading metadata for " << metadata.topics_by_id.size()
            << " topics" << std::endl;
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
  std::cout << "Reading partition log from: " << log_path << std::endl;

  std::ifstream file(log_path, std::ios::binary);
  if (!file.is_open()) {
    std::cout << "Unable to open partition log file" << std::endl;
    return {};
  }

  std::cout << "Successfully opened partition log file" << std::endl;
  auto batches = readAllBatches(file);
  std::cout << "Read " << batches.size() << " record batches from partition log"
            << std::endl;

  return batches;
}

std::string
KafkaLogMetadataReader::getPartitionLogPath(const std::string &topic_name,
                                            int32_t partition_id) {
  return base_path + "/" + topic_name + "-" + std::to_string(partition_id) +
         "/" + LOG_FILE;
}

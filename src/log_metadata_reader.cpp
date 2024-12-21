#include "include/log_metadata_reader.hpp"
#include <iostream>
#include <optional>

KafkaLogMetadataReader::KafkaLogMetadataReader(const std::string &log_path)
    : log_path(log_path) {}

std::vector<RecordBatchReader::RecordBatch>
KafkaLogMetadataReader::readAllBatches(std::ifstream &file) {
  std::cout << "Starting to read all record batches" << std::endl;
  std::vector<RecordBatchReader::RecordBatch> batches;

  size_t batch_count = 0;
  while (file.good() && !file.eof()) {
    std::cout << "Reading batch #" << batch_count << " at position "
              << file.tellg() << std::endl;

    // Check if we have more data to read
    if (file.peek() == EOF) {
      std::cout << "Reached end of file" << std::endl;
      break;
    }

    auto batch = RecordBatchReader(file)
                     .readHeader()
                     .readMetadata()
                     .readRecords()
                     .complete();

    std::cout << "Successfully read batch #" << batch_count << " with "
              << batch.records.size() << " records"
              << " at offset " << batch.base_offset << std::endl;

    batches.push_back(std::move(batch));
    batch_count++;
  }

  std::cout << "Finished reading " << batches.size() << " batches" << std::endl;
  return batches;
}

std::optional<KafkaLogMetadataReader::TopicMetadata>
KafkaLogMetadataReader::findTopic(const std::string &topic_name,
                                  std::optional<int> partition_id) {
  std::cout << "Searching for topic: " << topic_name << std::endl;
  std::ifstream file(log_path, std::ios::binary);

  if (!file.is_open()) {
    std::cout << "Failed to open log file: " << log_path << std::endl;
    return std::nullopt;
  }

  TopicMetadata result;

  auto batches = readAllBatches(file);
  bool found_topic = false;

  for (const auto &batch : batches) {
    for (const auto &record : batch.records) {
      // Check for topic record (type 0x02)
      if (record.value[1] == 0x02) {
        auto topic = parseTopicMetadata(record);
        if (topic.name == topic_name) {
          result = topic;
          found_topic = true;
        }
      }
      // Check for partition record (type 0x03)
      else if (found_topic && record.value[1] == 0x03) {
        auto partition = parsePartitionMetadata(record);
        if (partition.topic_id == result.topic_id) {
          result.partitions.push_back(partition);
        }
      }
    }
  }

  std::cout << "Found " << result.partitions.size()
            << " partitions for topic: " << topic_name << std::endl;
  if (found_topic) {
    return result;
  }

  return std::nullopt;
}

KafkaLogMetadataReader::TopicMetadata
KafkaLogMetadataReader::parseTopicMetadata(const RecordReader::Record &record) {
  TopicMetadata metadata;
  const uint8_t *data = record.value.data();

  // Skip frame version, type, version (3 bytes)
  size_t pos = 3;

  // Read name length (compact string format)
  uint8_t name_length = data[pos] - 1;
  pos++;

  // Read topic name
  metadata.name =
      std::string(reinterpret_cast<const char *>(data + pos), name_length);
  pos += name_length;

  // Read topic UUID (16 bytes)
  std::copy(data + pos, data + pos + 16, metadata.topic_id.begin());

  return metadata;
};

KafkaLogMetadataReader::PartitionMetadata
KafkaLogMetadataReader::parsePartitionMetadata(
    const RecordReader::Record &record) {
  PartitionMetadata metadata;
  const uint8_t *data = record.value.data();

  // Skip frame version, type, version
  size_t pos = 3;

  // Read partition ID (4 bytes)
  metadata.partition_id = (data[pos] << 24) | (data[pos + 1] << 16) |
                          (data[pos + 2] << 8) | data[pos + 3];
  pos += 4;

  // Read topic UUID (16 bytes)
  std::copy(data + pos, data + pos + 16, metadata.topic_id.begin());
  pos += 16;

  // Parse replicas array
  uint8_t replica_count = data[pos++] - 1; // Compact array length
  for (int i = 0; i < replica_count; i++) {
    int replica_id = (data[pos] << 24) | (data[pos + 1] << 16) |
                     (data[pos + 2] << 8) | data[pos + 3];
    metadata.replicas.push_back(replica_id);
    pos += 4;
  }

  // Parse ISR array similarly
  uint8_t isr_count = data[pos++] - 1;
  for (int i = 0; i < isr_count; i++) {
    int isr_id = (data[pos] << 24) | (data[pos + 1] << 16) |
                 (data[pos + 2] << 8) | data[pos + 3];
    metadata.isr.push_back(isr_id);
    pos += 4;
  }

  // Skip removing and adding replicas arrays
  pos += 2; // Skip compact array lengths

  // Read leader ID, leader epoch, partition epoch
  metadata.leader_id = (data[pos] << 24) | (data[pos + 1] << 16) |
                       (data[pos + 2] << 8) | data[pos + 3];
  pos += 4;

  metadata.leader_epoch = (data[pos] << 24) | (data[pos + 1] << 16) |
                          (data[pos + 2] << 8) | data[pos + 3];
  pos += 4;

  metadata.partition_epoch = (data[pos] << 24) | (data[pos + 1] << 16) |
                             (data[pos + 2] << 8) | data[pos + 3];

  return metadata;
}

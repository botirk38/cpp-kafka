#include "include/log_metadata_reader.hpp"
#include <iostream>

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

  auto batches = readAllBatches(file);
  std::cout << "Read " << batches.size() << " record batches" << std::endl;

  for (const auto &batch : batches) {
    std::cout << "Processing batch with base offset: " << batch.base_offset
              << std::endl;
    std::cout << "Batch contains " << batch.records.size() << " records"
              << std::endl;

    for (const auto &record : batch.records) {
      std::cout << "Processing record with length: " << record.length
                << ", key size: " << record.key.size()
                << ", value size: " << record.value.size() << std::endl;

      auto metadata = parseTopicMetadata(record);
      std::cout << "Parsed metadata - name: " << metadata.name
                << ", partition: " << metadata.partition_id << std::endl;

      if (metadata.name == topic_name) {
        std::cout << "Found matching topic: " << topic_name << std::endl;

        if (partition_id >= 0 && metadata.partition_id != partition_id) {
          continue;
        }
        return metadata;
      }
    }
  }

  std::cout << "Topic not found: " << topic_name << std::endl;
  return std::nullopt;
}

KafkaLogMetadataReader::TopicMetadata
KafkaLogMetadataReader::parseTopicMetadata(const RecordReader::Record &record) {
  TopicMetadata metadata;
  const uint8_t *data = record.value.data();
  size_t size = record.value.size();

  // Need at least frame version (1) + type (1) + version (1) bytes
  if (size < 3)
    return metadata;

  // Skip frame version, type, and record version
  size_t pos = 3;

  // Read name length (compact string format)
  if (size < pos + 1)
    return metadata;
  uint8_t name_length =
      data[pos] - 1; // Compact string length is encoded as actual length + 1
  pos++;

  // Read name
  if (size < pos + name_length)
    return metadata;
  metadata.name =
      std::string(reinterpret_cast<const char *>(data + pos), name_length);
  pos += name_length;

  // Read topic UUID
  if (size < pos + 16)
    return metadata;
  std::copy(data + pos, data + pos + 16, metadata.topic_id.begin());
  pos += 16;

  // For partition records, read partition ID
  if (data[1] == 0x03) { // Type 3 is partition record
    if (size < pos + 4)
      return metadata;
    metadata.partition_id = (data[pos] << 24) | (data[pos + 1] << 16) |
                            (data[pos + 2] << 8) | data[pos + 3];
  }

  return metadata;
}

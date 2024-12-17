#include "include/log_metadata_reader.hpp"

KafkaLogMetadataReader::KafkaLogMetadataReader(const std::string &log_path)
    : log_path(log_path) {}

std::vector<RecordBatchReader::RecordBatch>
KafkaLogMetadataReader::readAllBatches(std::ifstream &file) {
  std::vector<RecordBatchReader::RecordBatch> batches;

  while (file.good() && !file.eof()) {

    auto batch = RecordBatchReader(file)
                     .readHeader()
                     .readMetadata()
                     .readRecords()
                     .complete();

    batches.push_back(std::move(batch));
  }

  return batches;
}

std::optional<KafkaLogMetadataReader::TopicMetadata>
KafkaLogMetadataReader::findTopic(const std::string &topic_name) {
  std::ifstream file(log_path, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Failed to open metadata log file");
  }

  auto batches = readAllBatches(file);

  for (const auto &batch : batches) {
    for (const auto &record : batch.records) {
      if (record.key.size() >= 16) {
        auto metadata = parseTopicMetadata(record);
        if (metadata.name == topic_name) {
          return metadata;
        }
      }
    }
  }

  return std::nullopt;
}

KafkaLogMetadataReader::TopicMetadata
KafkaLogMetadataReader::parseTopicMetadata(const RecordReader::Record &record) {
  TopicMetadata metadata;
  std::copy(record.key.begin(), record.key.begin() + 16,
            metadata.topic_id.begin());

  if (!record.value.empty()) {
    const uint8_t *ptr = record.value.data();
    ptr += 2; // Skip version and type

    int16_t name_length = ntohs(*reinterpret_cast<const int16_t *>(ptr));
    ptr += 2;

    metadata.name =
        std::string(reinterpret_cast<const char *>(ptr), name_length);
    ptr += name_length;

    metadata.partition_id = ntohl(*reinterpret_cast<const int32_t *>(ptr));
  }

  return metadata;
}

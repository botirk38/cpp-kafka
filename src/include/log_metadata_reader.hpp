

#include "record_batch_reader.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <string>

class KafkaLogMetadataReader {

public:
  struct TopicMetadata {

    std::array<uint8_t, 16> topic_id;
    std::string name;
    int32_t partition_id;
  };

  explicit KafkaLogMetadataReader(const std::string &log_path);

  std::optional<TopicMetadata> findTopic(const std::string &topic_name,
                                         std::optional<int> partition_id = -1);

private:
  std::string log_path;

  TopicMetadata parseTopicMetadata(const RecordReader::Record &record);
  std::vector<RecordBatchReader::RecordBatch>
  readAllBatches(std::ifstream &file);
};

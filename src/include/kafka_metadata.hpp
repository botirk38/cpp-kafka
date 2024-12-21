#pragma once

#include "array"
#include "cstdint"
#include "vector"
#include <string>

namespace KafkaMetadata {
struct PartitionMetadata {
  int partition_id;
  std::array<uint8_t, 16> topic_id;
  int leader_id;
  int leader_epoch;
  int partition_epoch;
  std::vector<int> replicas;
  std::vector<int> isr; // in-sync replicas
};
struct TopicMetadata {

  std::array<uint8_t, 16> topic_id;
  std::string name;
  std::vector<PartitionMetadata> partitions;
};
} // namespace KafkaMetadata

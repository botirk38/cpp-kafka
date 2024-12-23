#pragma once

#include "array"
#include "cstdint"
#include "vector"
#include <string>

using uint128_t = __uint128_t;

namespace KafkaMetadata {
struct PartitionMetadata {
  int partition_id;
  uint128_t topic_id;
  int leader_id;
  int leader_epoch;
  int partition_epoch;
  std::vector<int> replicas;
  std::vector<int> isr; // in-sync replicas
};
struct TopicMetadata {

  uint128_t topic_id;
  std::string name;
  std::vector<PartitionMetadata> partitions;
};
} // namespace KafkaMetadata

#pragma once

#include "record_batch_reader.hpp"
#include <map>
#include <string>
#include <vector>

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
  std::vector<RecordBatchReader::RecordBatch> record_batches;
};
struct TopicMetadata {

  uint128_t topic_id;
  std::string name;
  std::vector<PartitionMetadata> partitions;
};

struct ClusterMetadata {
  std::map<uint128_t, TopicMetadata> topics_by_id;
};
} // namespace KafkaMetadata

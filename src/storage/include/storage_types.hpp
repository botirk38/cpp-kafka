#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

using uint128_t = __uint128_t;

namespace storage {

struct TopicId {
  uint128_t value{0};
  bool operator==(const TopicId &o) const { return value == o.value; }
  bool operator<(const TopicId &o) const { return value < o.value; }
};

struct PartitionInfo {
  int32_t partition_id{0};
  TopicId topic_id;
  int32_t leader_id{0};
  int32_t leader_epoch{0};
  int32_t partition_epoch{0};
  std::vector<int32_t> replicas;
  std::vector<int32_t> isr;
};

struct TopicInfo {
  TopicId topic_id;
  std::string name;
  std::vector<PartitionInfo> partitions;
};

struct ClusterSnapshot {
  std::map<TopicId, TopicInfo> topics_by_id;
};

// Raw bytes for one record batch (Kafka log format); used by fetch response
using RecordBatchBytes = std::vector<uint8_t>;

// All record batches for a partition
using PartitionData = std::vector<RecordBatchBytes>;

} // namespace storage

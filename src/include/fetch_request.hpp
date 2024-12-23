#pragma once

#include "kafka_request.hpp"
#include <array>
#include <vector>

struct FetchPartition {
  int32_t partition;
  int32_t current_leader_epoch;
  int64_t fetch_offset;
  int32_t last_fetched_epoch;
  int64_t log_start_offset;
  int32_t partition_max_bytes;
};

struct FetchTopic {
  std::array<uint8_t, 16> topic_id; // UUID
  std::vector<FetchPartition> partitions;
};

struct ForgottenTopic {
  std::array<uint8_t, 16> topic_id; // UUID
  std::vector<int32_t> partitions;
};

class FetchRequest : public KafkaRequest {
public:
  static constexpr int16_t KEY = 1; // Fetch API key

  int32_t max_wait_ms;
  int32_t min_bytes;
  int32_t max_bytes;
  int8_t isolation_level;
  int32_t session_id;
  int32_t session_epoch;
  std::vector<FetchTopic> topics;
  std::vector<ForgottenTopic> forgotten_topics_data;
  std::string rack_id;
};

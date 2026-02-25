#pragma once

#include "kafka_metadata.hpp"
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace KafkaMetadata {

class DecodeError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

TopicMetadata decodeTopicRecord(std::span<const uint8_t> data);
PartitionMetadata decodePartitionRecord(std::span<const uint8_t> data);

} // namespace KafkaMetadata

#pragma once

#include "storage_error.hpp"
#include "storage_types.hpp"
#include <span>

namespace storage::metadata {

TopicInfo decodeTopicRecord(std::span<const uint8_t> data);
PartitionInfo decodePartitionRecord(std::span<const uint8_t> data);

} // namespace storage::metadata

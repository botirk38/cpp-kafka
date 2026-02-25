#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace storage::metadata {

// Extract record values from a raw record batch (for metadata log parsing)
std::vector<std::vector<uint8_t>> extractRecordValues(std::span<const uint8_t> batch);

} // namespace storage::metadata

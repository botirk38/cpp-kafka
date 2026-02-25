#include "metadata/record_extractor.hpp"

namespace storage::metadata {

namespace {
int32_t readVarint(std::span<const uint8_t> &data) {
  int32_t value = 0;
  int shift = 0;
  while (!data.empty()) {
    uint8_t byte = data[0];
    data = data.subspan(1);
    value |= static_cast<int32_t>(byte & 0x7f) << shift;
    if (!(byte & 0x80)) {
      break;
    }
    shift += 7;
  }
  return value;
}

int32_t readZigZagVarint(std::span<const uint8_t> &data) {
  int32_t n = readVarint(data);
  return (n >> 1) ^ -(n & 1);
}

void skipRecord(std::span<const uint8_t> &data) {
  if (data.size() < 1) {
    return;
  }
  int32_t length = readVarint(data);
  if (length <= 0 || data.size() < static_cast<size_t>(length)) {
    return;
  }
  data = data.subspan(length);
}
} // namespace

std::vector<std::vector<uint8_t>> extractRecordValues(std::span<const uint8_t> batch) {
  std::vector<std::vector<uint8_t>> values;
  if (batch.size() < 12) {
    return values;
  }
  std::span<const uint8_t> rest = batch.subspan(12);

  constexpr size_t MAX_RECORDS = 10000;
  constexpr size_t MAX_FIELD = 1024 * 1024;

  while (!rest.empty() && values.size() < MAX_RECORDS) {
    if (rest.size() < 1) {
      break;
    }
    auto rec_start = rest;
    int32_t length = readVarint(rest);
    if (length <= 0 || rest.size() < static_cast<size_t>(length)) {
      break;
    }
    auto rec_data = rest.subspan(0, length);
    rest = rest.subspan(length);

    // Skip: attributes (1), timestamp_delta (varint), offset_delta (varint)
    if (rec_data.size() < 1) {
      continue;
    }
    rec_data = rec_data.subspan(1);
    readZigZagVarint(rec_data);
    readZigZagVarint(rec_data);

    // Key: zigzag varint length + bytes
    if (rec_data.empty()) {
      continue;
    }
    int32_t key_len = readZigZagVarint(rec_data);
    if (key_len < 0 || key_len > static_cast<int32_t>(MAX_FIELD) ||
        rec_data.size() < static_cast<size_t>(key_len)) {
      continue;
    }
    rec_data = rec_data.subspan(key_len);

    // Value: zigzag varint length + bytes
    if (rec_data.empty()) {
      continue;
    }
    int32_t value_len = readZigZagVarint(rec_data);
    if (value_len < 0 || value_len > static_cast<int32_t>(MAX_FIELD) ||
        rec_data.size() < static_cast<size_t>(value_len)) {
      continue;
    }
    std::vector<uint8_t> value(rec_data.begin(), rec_data.begin() + value_len);
    values.push_back(std::move(value));
  }

  return values;
}

} // namespace storage::metadata

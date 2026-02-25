#include "metadata/metadata_decoder.hpp"
#include <cstdint>

namespace storage::metadata {

namespace {
void checkBounds(std::span<const uint8_t> data, size_t pos, size_t need, const char *field) {
  if (pos + need > data.size()) {
    throw StorageError(ErrorCode::DecodeError,
                       std::string("Metadata decode: ") + field + " extends past buffer end");
  }
}

int32_t readInt32BE(std::span<const uint8_t> data, size_t &pos) {
  checkBounds(data, pos, 4, "int32");
  int32_t v = static_cast<int32_t>((data[pos] << 24) | (data[pos + 1] << 16) |
                                   (data[pos + 2] << 8) | data[pos + 3]);
  pos += 4;
  return v;
}

uint128_t readUint128(std::span<const uint8_t> data, size_t &pos) {
  checkBounds(data, pos, 16, "uuid");
  uint128_t v = 0;
  for (int i = 0; i < 16; i++) {
    v = (v << 8) | data[pos + i];
  }
  pos += 16;
  return v;
}
} // namespace

TopicInfo decodeTopicRecord(std::span<const uint8_t> data) {
  constexpr size_t HEADER_SIZE = 3;
  if (data.size() < HEADER_SIZE) {
    throw StorageError(ErrorCode::DecodeError, "Topic record too short for header");
  }

  size_t pos = HEADER_SIZE;

  checkBounds(data, pos, 1, "name_length");
  int name_len = static_cast<int>(data[pos]) - 1;
  pos++;
  if (name_len < 0) {
    throw StorageError(ErrorCode::DecodeError, "Invalid topic name length");
  }
  size_t name_length = static_cast<size_t>(name_len);

  if (name_length > data.size() - pos) {
    throw StorageError(ErrorCode::DecodeError, "Topic name extends past buffer");
  }
  std::string name(reinterpret_cast<const char *>(data.data() + pos), name_length);
  pos += name_length;

  TopicInfo info;
  info.name = std::move(name);
  info.topic_id = TopicId{readUint128(data, pos)};
  return info;
}

PartitionInfo decodePartitionRecord(std::span<const uint8_t> data) {
  constexpr size_t HEADER_SIZE = 3;
  if (data.size() < HEADER_SIZE) {
    throw StorageError(ErrorCode::DecodeError, "Partition record too short for header");
  }

  size_t pos = HEADER_SIZE;

  PartitionInfo info;
  info.partition_id = readInt32BE(data, pos);
  info.topic_id = TopicId{readUint128(data, pos)};

  checkBounds(data, pos, 1, "replica_count");
  int replica_count = static_cast<int>(data[pos++]) - 1;
  if (replica_count < 0 || pos + static_cast<size_t>(replica_count) * 4 > data.size()) {
    throw StorageError(ErrorCode::DecodeError, "Invalid replica count or buffer too short");
  }
  for (int i = 0; i < replica_count; i++) {
    info.replicas.push_back(readInt32BE(data, pos));
  }

  checkBounds(data, pos, 1, "isr_count");
  int isr_count = static_cast<int>(data[pos++]) - 1;
  if (isr_count < 0 || pos + static_cast<size_t>(isr_count) * 4 > data.size()) {
    throw StorageError(ErrorCode::DecodeError, "Invalid isr count or buffer too short");
  }
  for (int i = 0; i < isr_count; i++) {
    info.isr.push_back(readInt32BE(data, pos));
  }

  pos += 2; // Skip removing and adding replicas arrays

  info.leader_id = readInt32BE(data, pos);
  info.leader_epoch = readInt32BE(data, pos);
  info.partition_epoch = readInt32BE(data, pos);

  return info;
}

} // namespace storage::metadata

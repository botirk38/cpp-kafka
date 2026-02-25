#include "include/metadata_decoder.hpp"
#include <cstdint>

namespace KafkaMetadata {

namespace {
constexpr size_t MIN_TOPIC_RECORD_SIZE =
    3 + 1 + 1 + 16; // version, type, name_len, name_min, topic_id
constexpr size_t MIN_PARTITION_RECORD_SIZE = 3 + 4 + 16 + 1 + 1 + 2 + 4 + 4 + 4; // minimal fields

void checkBounds(std::span<const uint8_t> data, size_t pos, size_t need, const char *field) {
  if (pos + need > data.size()) {
    throw DecodeError(std::string("Metadata decode: ") + field + " extends past buffer end");
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

TopicMetadata decodeTopicRecord(std::span<const uint8_t> data) {
  constexpr size_t HEADER_SIZE = 3;
  if (data.size() < HEADER_SIZE) {
    throw DecodeError("Topic record too short for header");
  }

  size_t pos = HEADER_SIZE; // Skip frame version, type, version

  checkBounds(data, pos, 1, "name_length");
  int name_len = static_cast<int>(data[pos]) - 1;
  pos++;
  if (name_len < 0) {
    throw DecodeError("Invalid topic name length");
  }
  size_t name_length = static_cast<size_t>(name_len);

  if (name_length > data.size() - pos) {
    throw DecodeError("Topic name extends past buffer");
  }
  std::string name(reinterpret_cast<const char *>(data.data() + pos), name_length);
  pos += name_length;

  TopicMetadata metadata;
  metadata.name = std::move(name);
  metadata.topic_id = readUint128(data, pos);

  return metadata;
}

PartitionMetadata decodePartitionRecord(std::span<const uint8_t> data) {
  constexpr size_t HEADER_SIZE = 3;
  if (data.size() < HEADER_SIZE) {
    throw DecodeError("Partition record too short for header");
  }

  size_t pos = HEADER_SIZE;

  PartitionMetadata metadata;
  metadata.partition_id = readInt32BE(data, pos);
  metadata.topic_id = readUint128(data, pos);

  checkBounds(data, pos, 1, "replica_count");
  int replica_count = static_cast<int>(data[pos++]) - 1;
  if (replica_count < 0 || pos + static_cast<size_t>(replica_count) * 4 > data.size()) {
    throw DecodeError("Invalid replica count or buffer too short");
  }
  for (int i = 0; i < replica_count; i++) {
    metadata.replicas.push_back(readInt32BE(data, pos));
  }

  checkBounds(data, pos, 1, "isr_count");
  int isr_count = static_cast<int>(data[pos++]) - 1;
  if (isr_count < 0 || pos + static_cast<size_t>(isr_count) * 4 > data.size()) {
    throw DecodeError("Invalid isr count or buffer too short");
  }
  for (int i = 0; i < isr_count; i++) {
    metadata.isr.push_back(readInt32BE(data, pos));
  }

  pos += 2; // Skip removing and adding replicas arrays

  metadata.leader_id = readInt32BE(data, pos);
  metadata.leader_epoch = readInt32BE(data, pos);
  metadata.partition_epoch = readInt32BE(data, pos);

  return metadata;
}

} // namespace KafkaMetadata

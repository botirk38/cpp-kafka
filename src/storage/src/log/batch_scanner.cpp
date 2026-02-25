#include "log/batch_scanner.hpp"
#include <cstdio>

namespace storage::log {

BatchScanner::BatchScanner(std::ifstream &file) : cursor_(file) {}

std::vector<RecordBatchBytes> BatchScanner::scanAll() {
  std::vector<RecordBatchBytes> batches;
  while (cursor_.good() && !cursor_.eof()) {
    if (cursor_.peek() == EOF) {
      break;
    }
    auto batch = scanOne();
    if (!batch) {
      break;
    }
    batches.push_back(std::move(*batch));
  }
  return batches;
}

std::optional<RecordBatchBytes> BatchScanner::scanOne() {
  auto start = cursor_.tell();

  int64_t base_offset;
  int32_t batch_length;
  int32_t partition_leader_epoch;
  int8_t magic;
  uint32_t crc;

  cursor_.readInt64(base_offset)
      .readInt32(batch_length)
      .readInt32(partition_leader_epoch)
      .readRaw(magic)
      .readUint32(crc);

  if (!cursor_.good()) {
    return std::nullopt;
  }

  cursor_.seek(start);
  RecordBatchBytes raw(12 + batch_length);
  cursor_.readBytes(raw.data(), raw.size());
  if (!cursor_.good()) {
    return std::nullopt;
  }

  return raw;
}

} // namespace storage::log

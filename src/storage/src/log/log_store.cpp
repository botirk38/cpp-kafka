#include "log/log_store.hpp"
#include "log/batch_scanner.hpp"
#include <fstream>

namespace storage::log {

LogStore::LogStore(io::PathResolver resolver) : resolver_(std::move(resolver)) {}

std::expected<PartitionData, StorageError> LogStore::readPartition(const std::string &topic_name,
                                                                   int32_t partition_id) {
  std::ifstream file(resolver_.partitionLogPath(topic_name, partition_id), std::ios::binary);
  if (!file.is_open()) {
    return PartitionData{};
  }

  BatchScanner scanner(file);
  return scanner.scanAll();
}

} // namespace storage::log

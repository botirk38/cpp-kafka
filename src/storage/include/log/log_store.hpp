#pragma once

#include "io/path_resolver.hpp"
#include "storage_error.hpp"
#include "storage_types.hpp"
#include <expected>
#include <fstream>

namespace storage::log {

class LogStore {
public:
  explicit LogStore(io::PathResolver resolver);

  std::expected<PartitionData, StorageError> readPartition(const std::string &topic_name,
                                                           int32_t partition_id);

private:
  io::PathResolver resolver_;
};

} // namespace storage::log

#pragma once

#include "io/path_resolver.hpp"
#include "storage_error.hpp"
#include "storage_types.hpp"
#include <expected>
#include <fstream>

namespace storage::metadata {

class MetadataStore {
public:
  explicit MetadataStore(io::PathResolver resolver);

  std::expected<ClusterSnapshot, StorageError> loadClusterSnapshot();

private:
  io::PathResolver resolver_;
  static constexpr uint8_t TOPIC_RECORD = 0x02;
  static constexpr uint8_t PARTITION_RECORD = 0x03;
};

} // namespace storage::metadata

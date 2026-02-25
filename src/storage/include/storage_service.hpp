#pragma once

#include "storage_error.hpp"
#include "storage_types.hpp"
#include <expected>
#include <memory>
#include <optional>
#include <string>

namespace storage {

class IStorageService {
public:
  virtual ~IStorageService() = default;

  // Load full cluster metadata snapshot
  virtual std::expected<ClusterSnapshot, StorageError> loadClusterSnapshot() = 0;

  // Lookup topic by name (from snapshot)
  virtual std::optional<TopicInfo> findTopicByName(const ClusterSnapshot &snapshot,
                                                   const std::string &name) const = 0;

  // Lookup topic by id (from snapshot)
  virtual std::optional<TopicInfo> findTopicById(const ClusterSnapshot &snapshot,
                                                 TopicId id) const = 0;

  // Read partition log data (raw record batches)
  virtual std::expected<PartitionData, StorageError>
  readPartitionData(const std::string &topic_name, int32_t partition_id) = 0;
};

std::unique_ptr<IStorageService> createStorageService(std::string base_path);

} // namespace storage

#pragma once

#include "log/log_store.hpp"
#include "metadata/metadata_store.hpp"
#include "storage_service.hpp"
#include "storage_types.hpp"
#include <string>

namespace storage::internal {

class StorageServiceImpl : public IStorageService {
public:
  explicit StorageServiceImpl(std::string base_path);

  std::expected<ClusterSnapshot, StorageError> loadClusterSnapshot() override;

  std::optional<TopicInfo> findTopicByName(const ClusterSnapshot &snapshot,
                                           const std::string &name) const override;

  std::optional<TopicInfo> findTopicById(const ClusterSnapshot &snapshot,
                                         TopicId id) const override;

  std::expected<PartitionData, StorageError> readPartitionData(const std::string &topic_name,
                                                               int32_t partition_id) override;

private:
  io::PathResolver path_resolver_;
  metadata::MetadataStore metadata_store_;
  log::LogStore log_store_;
};

} // namespace storage::internal

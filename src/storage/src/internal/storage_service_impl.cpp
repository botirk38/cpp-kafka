#include "internal/storage_service_impl.hpp"
#include <algorithm>

namespace storage::internal {

StorageServiceImpl::StorageServiceImpl(std::string base_path)
    : path_resolver_(std::move(base_path)), metadata_store_(path_resolver_),
      log_store_(path_resolver_) {}

std::expected<ClusterSnapshot, StorageError> StorageServiceImpl::loadClusterSnapshot() {
  return metadata_store_.loadClusterSnapshot();
}

std::optional<TopicInfo> StorageServiceImpl::findTopicByName(const ClusterSnapshot &snapshot,
                                                             const std::string &name) const {
  auto it = std::find_if(snapshot.topics_by_id.begin(), snapshot.topics_by_id.end(),
                         [&name](const auto &p) { return p.second.name == name; });
  if (it != snapshot.topics_by_id.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::optional<TopicInfo> StorageServiceImpl::findTopicById(const ClusterSnapshot &snapshot,
                                                           TopicId id) const {
  auto it = snapshot.topics_by_id.find(id);
  if (it != snapshot.topics_by_id.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::expected<PartitionData, StorageError>
StorageServiceImpl::readPartitionData(const std::string &topic_name, int32_t partition_id) {
  return log_store_.readPartition(topic_name, partition_id);
}

} // namespace storage::internal

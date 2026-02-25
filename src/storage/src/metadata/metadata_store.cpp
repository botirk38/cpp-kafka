#include "metadata/metadata_store.hpp"
#include "log/batch_scanner.hpp"
#include "metadata/metadata_decoder.hpp"
#include "metadata/record_extractor.hpp"
#include <fstream>

namespace storage::metadata {

MetadataStore::MetadataStore(io::PathResolver resolver) : resolver_(std::move(resolver)) {}

std::expected<ClusterSnapshot, StorageError> MetadataStore::loadClusterSnapshot() {
  ClusterSnapshot snapshot;
  std::ifstream file(resolver_.clusterMetadataPath(), std::ios::binary);
  if (!file.is_open()) {
    return snapshot;
  }

  try {
    log::BatchScanner scanner(file);
    auto batches = scanner.scanAll();

    for (const auto &batch : batches) {
      auto values = extractRecordValues(batch);
      for (const auto &value : values) {
        if (value.size() < 2) {
          continue;
        }
        if (value[1] == TOPIC_RECORD) {
          auto topic = decodeTopicRecord(value);
          TopicInfo &info = snapshot.topics_by_id[topic.topic_id];
          info = std::move(topic);
        } else if (value[1] == PARTITION_RECORD) {
          auto partition = decodePartitionRecord(value);
          auto it = snapshot.topics_by_id.find(partition.topic_id);
          if (it != snapshot.topics_by_id.end()) {
            it->second.partitions.push_back(std::move(partition));
          }
        }
      }
    }
  } catch (const StorageError &e) {
    return std::unexpected(e);
  }

  return snapshot;
}

} // namespace storage::metadata

#include "io/path_resolver.hpp"

namespace storage::io {

std::string PathResolver::clusterMetadataPath() const {
  return base_path_ + "/__cluster_metadata-0/" + LOG_FILE;
}

std::string PathResolver::partitionLogPath(const std::string &topic_name,
                                           int32_t partition_id) const {
  return base_path_ + "/" + topic_name + "-" + std::to_string(partition_id) + "/" + LOG_FILE;
}

} // namespace storage::io

#pragma once

#include <string>

namespace storage::io {

class PathResolver {
public:
  explicit PathResolver(std::string base_path) : base_path_(std::move(base_path)) {}

  std::string clusterMetadataPath() const;
  std::string partitionLogPath(const std::string &topic_name, int32_t partition_id) const;

private:
  std::string base_path_;
  static constexpr const char *LOG_FILE = "00000000000000000000.log";
};

} // namespace storage::io
